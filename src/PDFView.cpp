#include "PDFView.h"

#include <ScrollBar.h>
#include <ScrollView.h>
#include <Font.h>
#include <InterfaceDefs.h>
#include <String.h>

#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-page-renderer.h>
#include <poppler-image.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdlib>

PDFView::PDFView(const char* name)
    : BView(name, B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE),
      fRenderedBitmap(nullptr),
      fZoom(1.0f),
      fCurrentPage(0),
      fTotalPages(0),
      fIsDragging(false),
      fLastMousePos(0, 0),
      fCurrentFilePath(nullptr)
{
}

PDFView::~PDFView()
{
    delete fRenderedBitmap;
    free(fCurrentFilePath);
}

void
PDFView::AttachedToWindow()
{
    BView::AttachedToWindow();
    SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
    _UpdateScrollBars();
}

void
PDFView::Draw(BRect updateRect)
{
    BRect bounds = Bounds();

    if (fRenderedBitmap == nullptr) {
        // Draw empty state placeholder
        SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
        FillRect(updateRect, B_SOLID_LOW);

        SetHighColor(tint_color(ui_color(B_PANEL_TEXT_COLOR), B_LIGHTEN_1_TINT));
        BFont font;
        GetFont(&font);
        font.SetSize(14.0f);
        SetFont(&font);

        const char* title = "Docrobat";
        const char* subtitle = "Nessun documento aperto. Usa File -> Apri... o trascina un file PDF.";

        float titleWidth = StringWidth(title);
        float subWidth = StringWidth(subtitle);

        BPoint titlePos(
            std::max(10.0f, (bounds.Width() - titleWidth) / 2.0f),
            std::max(30.0f, (bounds.Height() / 2.0f) - 15.0f)
        );
        BPoint subPos(
            std::max(10.0f, (bounds.Width() - subWidth) / 2.0f),
            std::max(50.0f, (bounds.Height() / 2.0f) + 15.0f)
        );

        DrawString(title, titlePos);
        font.SetSize(11.0f);
        SetFont(&font);
        DrawString(subtitle, subPos);
        return;
    }

    // Clear background around page
    SetLowColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_1_TINT));
    FillRect(updateRect, B_SOLID_LOW);

    BRect bmpBounds = fRenderedBitmap->Bounds();
    float x = 20.0f;
    if (bounds.Width() > bmpBounds.Width() + 40.0f)
        x = (bounds.Width() - bmpBounds.Width()) / 2.0f;

    float y = 20.0f;

    BRect pageRect(x, y, x + bmpBounds.Width(), y + bmpBounds.Height());

    // Draw drop shadow
    BRect shadowRect = pageRect;
    shadowRect.OffsetBy(4.0f, 4.0f);
    SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_3_TINT));
    FillRect(shadowRect);

    // Draw white paper base
    SetHighColor(255, 255, 255);
    FillRect(pageRect);

    // Render bitmap onto view
    DrawBitmap(fRenderedBitmap, pageRect);

    // Border around page
    SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_4_TINT));
    StrokeRect(pageRect);
}

void
PDFView::FrameResized(float newWidth, float newHeight)
{
    BView::FrameResized(newWidth, newHeight);
    _UpdateScrollBars();
    Invalidate();
}

void
PDFView::MouseDown(BPoint where)
{
    BMessage* currentMsg = Window()->CurrentMessage();
    int32 buttons = 0;
    if (currentMsg != nullptr)
        currentMsg->FindInt32("buttons", &buttons);

    if ((buttons & B_PRIMARY_MOUSE_BUTTON) != 0) {
        fIsDragging = true;
        fLastMousePos = where;
        SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
    }
    BView::MouseDown(where);
}

void
PDFView::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
    if (fIsDragging) {
        BPoint delta = where - fLastMousePos;
        ScrollBy(-delta.x, -delta.y);
        fLastMousePos = where - delta;
    }
    BView::MouseMoved(where, transit, dragMessage);
}

void
PDFView::MouseUp(BPoint where)
{
    if (fIsDragging)
        fIsDragging = false;
    BView::MouseUp(where);
}

void
PDFView::SetBitmap(BBitmap* bitmap)
{
    if (fRenderedBitmap != bitmap) {
        delete fRenderedBitmap;
        fRenderedBitmap = bitmap;
    }

    _UpdateScrollBars();
    Invalidate();
}

void
PDFView::SetZoom(float zoom)
{
    if (zoom < 0.25f)
        zoom = 0.25f;
    if (zoom > 5.0f)
        zoom = 5.0f;

    if (std::fabs(fZoom - zoom) > 0.001f) {
        fZoom = zoom;
        if (fCurrentFilePath != nullptr)
            _RenderCurrentPage();
    }
}

void
PDFView::SetPageInfo(int32 current, int32 total)
{
    fCurrentPage = current;
    fTotalPages = total;
}

bool
PDFView::LoadDocument(const char* filePath)
{
    if (filePath == nullptr)
        return false;

    free(fCurrentFilePath);
    fCurrentFilePath = strdup(filePath);

    std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(filePath));
    if (!doc || doc->is_locked())
        return false;

    fTotalPages = doc->pages();
    fCurrentPage = 0;

    _RenderCurrentPage();
    return true;
}

void
PDFView::NextPage()
{
    if (fCurrentPage + 1 < fTotalPages) {
        fCurrentPage++;
        _RenderCurrentPage();
    }
}

void
PDFView::PreviousPage()
{
    if (fCurrentPage > 0) {
        fCurrentPage--;
        _RenderCurrentPage();
    }
}

void
PDFView::_UpdateScrollBars()
{
    BScrollBar* h = ScrollBar(B_HORIZONTAL);
    BScrollBar* v = ScrollBar(B_VERTICAL);

    if (h == nullptr || v == nullptr)
        return;

    BRect bounds = Bounds();
    float contentWidth = 0.0f;
    float contentHeight = 0.0f;

    if (fRenderedBitmap != nullptr) {
        contentWidth = fRenderedBitmap->Bounds().Width() + 40.0f;
        contentHeight = fRenderedBitmap->Bounds().Height() + 40.0f;
    }

    float maxH = std::max(0.0f, contentWidth - bounds.Width());
    float maxV = std::max(0.0f, contentHeight - bounds.Height());

    h->SetRange(0.0f, maxH);
    v->SetRange(0.0f, maxV);

    h->SetProportion(contentWidth > 0.0f ? bounds.Width() / contentWidth : 1.0f);
    v->SetProportion(contentHeight > 0.0f ? bounds.Height() / contentHeight : 1.0f);
}

void
PDFView::_RenderCurrentPage()
{
    if (fCurrentFilePath == nullptr)
        return;

    std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(fCurrentFilePath));
    if (!doc || fCurrentPage < 0 || fCurrentPage >= doc->pages())
        return;

    std::unique_ptr<poppler::page> page(doc->create_page(fCurrentPage));
    if (!page)
        return;

    poppler::page_renderer renderer;
    renderer.set_image_format(poppler::image::format_argb32);

    double dpi = 72.0 * static_cast<double>(fZoom);
    poppler::image img = renderer.render_page(page.get(), dpi, dpi);

    if (!img.is_valid())
        return;

    BRect rect(0, 0, img.width() - 1, img.height() - 1);
    BBitmap* newBitmap = new BBitmap(rect, B_RGBA32);

    const char* src = img.data();
    uint8* dst = static_cast<uint8*>(newBitmap->Bits());
    int32 srcRowBytes = img.bytes_per_row();
    int32 dstRowBytes = newBitmap->BytesPerRow();
    int32 copyRowBytes = std::min(srcRowBytes, dstRowBytes);

    for (int y = 0; y < img.height(); ++y) {
        memcpy(dst + (y * dstRowBytes), src + (y * srcRowBytes), copyRowBytes);
    }

    SetBitmap(newBitmap);
}
