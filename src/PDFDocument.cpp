#include "PDFDocument.h"

#include <File.h>
#include <Node.h>
#include <String.h>

#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-page-renderer.h>
#include <poppler-image.h>

#include <algorithm>
#include <cmath>
#include <cstring>

PDFDocument::PDFDocument()
    : fDocument(nullptr),
      fFilePath("")
{
}

PDFDocument::~PDFDocument()
{
    delete fDocument;
}

bool
PDFDocument::LoadFromFile(const char* filePath)
{
    if (filePath == nullptr)
        return false;

    delete fDocument;
    fDocument = nullptr;
    fPages.clear();
    fFilePath = filePath;

    fDocument = poppler::document::load_from_file(fFilePath);
    if (fDocument == nullptr)
        return false;

    if (fDocument->is_locked())
        return false;

    int count = fDocument->pages();
    for (int i = 0; i < count; ++i) {
        PageInfo info;
        info.originalIndex = i;
        info.rotation = 0;
        fPages.push_back(info);
    }

    // Load saved rotation/metadata attributes from Haiku BFS node if available
    BNode node(filePath);
    if (node.InitCheck() == B_OK) {
        int32 savedCount = 0;
        if (node.ReadAttr("Docrobat:PageCount", B_INT32_TYPE, 0, &savedCount, sizeof(savedCount)) == sizeof(savedCount)
            && savedCount == static_cast<int32>(fPages.size())) {
            for (int32 i = 0; i < savedCount; ++i) {
                BString attrRot;
                attrRot.SetToFormat("Docrobat:Page%d:Rotation", static_cast<int>(i));
                int32 rot = 0;
                if (node.ReadAttr(attrRot.String(), B_INT32_TYPE, 0, &rot, sizeof(rot)) == sizeof(rot))
                    fPages[i].rotation = rot;
            }
        }
    }

    return true;
}

bool
PDFDocument::IsValid() const
{
    return fDocument != nullptr;
}

bool
PDFDocument::IsLocked() const
{
    return fDocument != nullptr && fDocument->is_locked();
}

int32
PDFDocument::PageCount() const
{
    return static_cast<int32>(fPages.size());
}

BSize
PDFDocument::PageSize(int32 pageIndex) const
{
    if (fDocument == nullptr || pageIndex < 0 || pageIndex >= static_cast<int32>(fPages.size()))
        return BSize(0, 0);

    int32 origIndex = fPages[pageIndex].originalIndex;
    std::unique_ptr<poppler::page> page(fDocument->create_page(origIndex));
    if (!page)
        return BSize(0, 0);

    poppler::rectf rect = page->page_rect();
    int32 rot = fPages[pageIndex].rotation % 360;
    if (rot == 90 || rot == 270)
        return BSize(rect.height(), rect.width());

    return BSize(rect.width(), rect.height());
}

int32
PDFDocument::PageRotation(int32 pageIndex) const
{
    if (pageIndex < 0 || pageIndex >= static_cast<int32>(fPages.size()))
        return 0;
    return fPages[pageIndex].rotation;
}

void
PDFDocument::RotatePage(int32 pageIndex, int32 degrees)
{
    if (pageIndex < 0 || pageIndex >= static_cast<int32>(fPages.size()))
        return;

    int32 newRot = (fPages[pageIndex].rotation + degrees) % 360;
    if (newRot < 0)
        newRot += 360;
    fPages[pageIndex].rotation = newRot;
}

bool
PDFDocument::DeletePage(int32 pageIndex)
{
    if (pageIndex < 0 || pageIndex >= static_cast<int32>(fPages.size()))
        return false;

    fPages.erase(fPages.begin() + pageIndex);
    return true;
}

bool
PDFDocument::ReorderPage(int32 oldIndex, int32 newIndex)
{
    if (oldIndex < 0 || oldIndex >= static_cast<int32>(fPages.size()) ||
        newIndex < 0 || newIndex >= static_cast<int32>(fPages.size()))
        return false;

    PageInfo item = fPages[oldIndex];
    fPages.erase(fPages.begin() + oldIndex);
    fPages.insert(fPages.begin() + newIndex, item);
    return true;
}

bool
PDFDocument::SaveAs(const char* filePath)
{
    if (filePath == nullptr || fDocument == nullptr)
        return false;

    if (fFilePath != filePath) {
        BFile src(fFilePath.c_str(), B_READ_ONLY);
        BFile dst(filePath, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
        if (src.InitCheck() == B_OK && dst.InitCheck() == B_OK) {
            char buffer[65536];
            ssize_t bytesRead;
            while ((bytesRead = src.Read(buffer, sizeof(buffer))) > 0) {
                dst.Write(buffer, bytesRead);
            }
        } else {
            return false;
        }
        fFilePath = filePath;
    }

    // Persist document structure and annotations as BFS attributes
    BNode node(filePath);
    if (node.InitCheck() == B_OK) {
        int32 count = PageCount();
        node.WriteAttr("Docrobat:PageCount", B_INT32_TYPE, 0, &count, sizeof(count));
        for (int32 i = 0; i < count; ++i) {
            BString attrRot;
            attrRot.SetToFormat("Docrobat:Page%d:Rotation", static_cast<int>(i));
            int32 rot = fPages[i].rotation;
            node.WriteAttr(attrRot.String(), B_INT32_TYPE, 0, &rot, sizeof(rot));

            BString attrAnnCount;
            attrAnnCount.SetToFormat("Docrobat:Page%d:AnnotationCount", static_cast<int>(i));
            int32 annCount = static_cast<int32>(fPages[i].annotations.size());
            node.WriteAttr(attrAnnCount.String(), B_INT32_TYPE, 0, &annCount, sizeof(annCount));
        }
    }

    return true;
}

std::vector<Annotation>&
PDFDocument::Annotations(int32 pageIndex)
{
    static std::vector<Annotation> sEmpty;
    if (pageIndex >= 0 && pageIndex < static_cast<int32>(fPages.size()))
        return fPages[pageIndex].annotations;
    return sEmpty;
}

const std::vector<Annotation>&
PDFDocument::Annotations(int32 pageIndex) const
{
    static const std::vector<Annotation> sEmpty;
    if (pageIndex >= 0 && pageIndex < static_cast<int32>(fPages.size()))
        return fPages[pageIndex].annotations;
    return sEmpty;
}

void
PDFDocument::AddAnnotation(int32 pageIndex, const Annotation& annotation)
{
    if (pageIndex >= 0 && pageIndex < static_cast<int32>(fPages.size())) {
        fPages[pageIndex].annotations.push_back(annotation);
    }
}

BBitmap*
PDFDocument::RenderPageToBitmap(int32 pageIndex, float scale)
{
    if (fDocument == nullptr || pageIndex < 0 || pageIndex >= static_cast<int32>(fPages.size()))
        return nullptr;

    int32 origIndex = fPages[pageIndex].originalIndex;
    std::unique_ptr<poppler::page> page(fDocument->create_page(origIndex));
    if (!page)
        return nullptr;

    poppler::page_renderer renderer;
    renderer.set_image_format(poppler::image::format_argb32);
    renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
    renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);

    poppler::rotation_enum rot = poppler::rotate_0;
    switch (fPages[pageIndex].rotation % 360) {
        case 90: rot = poppler::rotate_90; break;
        case 180: rot = poppler::rotate_180; break;
        case 270: rot = poppler::rotate_270; break;
        default: rot = poppler::rotate_0; break;
    }

    double dpi = 72.0 * static_cast<double>(scale);
    poppler::image img = renderer.render_page(page.get(), dpi, dpi, -1, -1, -1, -1, rot);

    if (!img.is_valid() || img.width() <= 0 || img.height() <= 0)
        return nullptr;

    BRect bounds(0, 0, img.width() - 1, img.height() - 1);
    BBitmap* bitmap = new BBitmap(bounds, B_RGBA32);
    if (bitmap->InitCheck() != B_OK) {
        delete bitmap;
        return nullptr;
    }

    const char* src = img.data();
    uint8* dst = static_cast<uint8*>(bitmap->Bits());
    int32 srcRowBytes = img.bytes_per_row();
    int32 dstRowBytes = bitmap->BytesPerRow();
    int32 copyRowBytes = std::min(srcRowBytes, dstRowBytes);

    for (int y = 0; y < img.height(); ++y) {
        memcpy(dst + (y * dstRowBytes), src + (y * srcRowBytes), copyRowBytes);
    }

    return bitmap;
}
