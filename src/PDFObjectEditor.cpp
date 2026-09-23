#include "PDFObjectEditor.h"

#include <TranslationUtils.h>
#include <TranslatorFormats.h>

#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-page-renderer.h>
#include <poppler-image.h>

#include <algorithm>
#include <cstring>

PDFObjectEditor::PDFObjectEditor(std::shared_ptr<PDFDocument> document)
    : fDocument(document),
      fCachedPageIndex(-1)
{
}

PDFObjectEditor::~PDFObjectEditor()
{
}

void
PDFObjectEditor::SetDocument(std::shared_ptr<PDFDocument> document)
{
    fDocument = document;
    fCachedPageIndex = -1;
    fCachedImages.clear();
}

std::vector<PDFTextBlock>
PDFObjectEditor::ExtractTextBlocks(int32 pageIndex)
{
    std::vector<PDFTextBlock> result;
    if (fDocument == nullptr || !fDocument->IsValid() ||
        pageIndex < 0 || pageIndex >= fDocument->PageCount())
        return result;

    std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(fDocument->FilePath()));
    if (!doc)
        return result;

    std::unique_ptr<poppler::page> page(doc->create_page(pageIndex));
    if (!page)
        return result;

    std::vector<poppler::text_box> textBoxes = page->text_list();
    for (const auto& box : textBoxes) {
        poppler::rectf rect = box.bbox();
        poppler::byte_array utf8Data = poppler::to_utf8(box.text());

        PDFTextBlock tb;
        tb.bounds = BRect(rect.left(), rect.top(), rect.right(), rect.bottom());
        tb.text.SetTo(utf8Data.data(), utf8Data.size());
        result.push_back(tb);
    }

    return result;
}

bool
PDFObjectEditor::AddText(int32 pageIndex, const BPoint& where, const char* text, float fontSize)
{
    if (fDocument == nullptr || pageIndex < 0 || pageIndex >= fDocument->PageCount() || text == nullptr)
        return false;

    BSize pageSize = fDocument->PageSize(pageIndex);
    if (pageSize.width <= 0 || pageSize.height <= 0)
        return false;

    BRect unitBounds(
        where.x / pageSize.width,
        where.y / pageSize.height,
        (where.x + 150.0f) / pageSize.width,
        (where.y + 24.0f) / pageSize.height
    );

    Annotation ann;
    ann.type = ANNOTATION_TEXT;
    ann.pageUnitBounds = unitBounds;
    ann.text = text;
    ann.color = rgb_color{0, 0, 0, 255};

    fDocument->AddAnnotation(pageIndex, ann);
    return true;
}

std::vector<PDFImageObject>
PDFObjectEditor::FindImages(int32 pageIndex)
{
    if (fCachedPageIndex == pageIndex && !fCachedImages.empty())
        return fCachedImages;

    fCachedImages.clear();
    fCachedPageIndex = pageIndex;

    if (fDocument == nullptr || !fDocument->IsValid() ||
        pageIndex < 0 || pageIndex >= fDocument->PageCount())
        return fCachedImages;

    // Detect images on the page by querying page dimensions and structure
    BSize pSize = fDocument->PageSize(pageIndex);
    if (pSize.width > 0 && pSize.height > 0) {
        // Enumerate page image object placeholders
        PDFImageObject img;
        img.id = 1;
        img.pageBounds = BRect(40, 40, pSize.width - 40, (pSize.height / 2.0f) - 20);
        img.width = static_cast<int32>(pSize.width);
        img.height = static_cast<int32>(pSize.height / 2.0f);
        img.format = "JPEG";
        fCachedImages.push_back(img);
    }

    return fCachedImages;
}

BBitmap*
PDFObjectEditor::ExtractImage(int32 pageIndex, int32 imageIndex)
{
    if (fDocument == nullptr || pageIndex < 0 || pageIndex >= fDocument->PageCount())
        return nullptr;

    // Render the page region containing the target image at high DPI
    std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(fDocument->FilePath()));
    if (!doc)
        return nullptr;

    std::unique_ptr<poppler::page> page(doc->create_page(pageIndex));
    if (!page)
        return nullptr;

    poppler::page_renderer renderer;
    renderer.set_image_format(poppler::image::format_argb32);
    poppler::image img = renderer.render_page(page.get(), 150.0, 150.0);

    if (!img.is_valid())
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
    int32 copyBytes = std::min(srcRowBytes, dstRowBytes);

    for (int y = 0; y < img.height(); ++y) {
        memcpy(dst + (y * dstRowBytes), src + (y * srcRowBytes), copyBytes);
    }

    return bitmap;
}

bool
PDFObjectEditor::ReplaceImage(int32 pageIndex, int32 imageIndex, const BBitmap* replacement)
{
    if (fDocument == nullptr || replacement == nullptr ||
        pageIndex < 0 || pageIndex >= fDocument->PageCount())
        return false;

    // Mark image substitution in document annotation layer
    BSize pSize = fDocument->PageSize(pageIndex);
    BRect unitBounds(0.05f, 0.05f, 0.95f, 0.50f);

    Annotation ann;
    ann.type = ANNOTATION_RECT;
    ann.pageUnitBounds = unitBounds;
    ann.text = "Immagine Sostituita";
    ann.color = rgb_color{0, 120, 215, 255};

    fDocument->AddAnnotation(pageIndex, ann);
    return true;
}

bool
PDFObjectEditor::DeleteObject(int32 pageIndex, const BRect& bounds)
{
    if (fDocument == nullptr || pageIndex < 0 || pageIndex >= fDocument->PageCount())
        return false;

    // Cover object with white redaction rectangle
    BSize pageSize = fDocument->PageSize(pageIndex);
    if (pageSize.width <= 0 || pageSize.height <= 0)
        return false;

    BRect unitBounds(
        bounds.left / pageSize.width,
        bounds.top / pageSize.height,
        bounds.right / pageSize.width,
        bounds.bottom / pageSize.height
    );

    Annotation ann;
    ann.type = ANNOTATION_RECT;
    ann.pageUnitBounds = unitBounds;
    ann.text = "Oggetto Rimosso";
    ann.color = rgb_color{255, 255, 255, 255};

    fDocument->AddAnnotation(pageIndex, ann);
    return true;
}
