#include "PDFDocument.h"

#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-page-renderer.h>
#include <poppler-image.h>

#include <algorithm>
#include <cstring>
#include <cmath>

PDFDocument::PDFDocument()
    : fDocument(nullptr),
      fFilePath(""),
      fPageCount(0)
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
    fPageCount = 0;
    fFilePath = filePath;

    fDocument = poppler::document::load_from_file(fFilePath);
    if (fDocument == nullptr)
        return false;

    if (fDocument->is_locked())
        return false;

    fPageCount = fDocument->pages();
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
    return fPageCount;
}

BSize
PDFDocument::PageSize(int32 pageIndex) const
{
    if (fDocument == nullptr || pageIndex < 0 || pageIndex >= fPageCount)
        return BSize(0, 0);

    std::unique_ptr<poppler::page> page(fDocument->create_page(pageIndex));
    if (!page)
        return BSize(0, 0);

    poppler::rectf rect = page->page_rect();
    return BSize(rect.width(), rect.height());
}

BBitmap*
PDFDocument::RenderPageToBitmap(int32 pageIndex, float scale)
{
    if (fDocument == nullptr || pageIndex < 0 || pageIndex >= fPageCount)
        return nullptr;

    std::unique_ptr<poppler::page> page(fDocument->create_page(pageIndex));
    if (!page)
        return nullptr;

    poppler::page_renderer renderer;
    renderer.set_image_format(poppler::image::format_argb32);
    renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
    renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);

    double dpi = 72.0 * static_cast<double>(scale);
    poppler::image img = renderer.render_page(page.get(), dpi, dpi);

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
