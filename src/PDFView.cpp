#include "PDFView.h"

#include <Font.h>
#include <InterfaceDefs.h>
#include <ScrollBar.h>
#include <ScrollView.h>
#include <String.h>
#include <Window.h>

#include <algorithm>
#include <cmath>

PDFView::PDFView(const char* name)
    : BView(name, B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE),
      fDocument(nullptr),
      fCurrentPage(0),
      fZoom(1.0f),
      fRenderedBitmap(nullptr),
      fIsDragging(false),
      fLastMousePos(0, 0)
{
}

PDFView::~PDFView()
{
    delete fRenderedBitmap;
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
        // Empty state view
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
            std::max(10.0f, bounds.left + (bounds.Width() - titleWidth) / 2.0f),
            std::max(30.0f, bounds.top + (bounds.Height() / 2.0f) - 15.0f)
        );
        BPoint subPos(
            std::max(10.0f, bounds.left + (bounds.Width() - subWidth) / 2.0f),
            std::max(50.0f, bounds.top + (bounds.Height() / 2.0f) + 15.0f)
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

    BRect pageRect = _CalculatePageRect();

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

    // Draw border around page
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
PDFView::KeyDown(const char* bytes, int32 numBytes)
{
    if (numBytes == 1) {
        switch (bytes[0]) {
            case B_PAGE_UP:
                PreviousPage();
                return;
            case B_PAGE_DOWN:
                NextPage();
                return;
            case B_HOME:
                FirstPage();
                return;
            case B_END:
                LastPage();
                return;
            case '+':
            case '=':
                SetZoom(fZoom * 1.25f);
                return;
            case '-':
                SetZoom(fZoom / 1.25f);
                return;
        }
    }
    BView::KeyDown(bytes, numBytes);
}

void
PDFView::SetDocument(std::shared_ptr<PDFDocument> document)
{
    fDocument = document;
    fCurrentPage = 0;
    _RenderCurrentPage();
}

bool
PDFView::LoadDocument(const char* filePath)
{
    if (filePath == nullptr)
        return false;

    auto doc = std::make_shared<PDFDocument>();
    if (!doc->LoadFromFile(filePath))
        return false;

    SetDocument(doc);
    return true;
}

void
PDFView::SetCurrentPage(int32 pageIndex)
{
    if (pageIndex < 0 || pageIndex >= PageCount())
        return;

    if (fCurrentPage != pageIndex) {
        fCurrentPage = pageIndex;
        _RenderCurrentPage();
    }
}

int32
PDFView::PageCount() const
{
    return fDocument ? fDocument->PageCount() : 0;
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
        _RenderCurrentPage();
    }
}

void
PDFView::NextPage()
{
    if (fCurrentPage + 1 < PageCount())
        SetCurrentPage(fCurrentPage + 1);
}

void
PDFView::PreviousPage()
{
    if (fCurrentPage > 0)
        SetCurrentPage(fCurrentPage - 1);
}

void
PDFView::FirstPage()
{
    if (PageCount() > 0)
        SetCurrentPage(0);
}

void
PDFView::LastPage()
{
    if (PageCount() > 0)
        SetCurrentPage(PageCount() - 1);
}

void
PDFView::RefreshPage()
{
    _RenderCurrentPage();
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

    h->SetSteps(20.0f, std::max(20.0f, bounds.Width() * 0.75f));
    v->SetSteps(20.0f, std::max(20.0f, bounds.Height() * 0.75f));
}

void
PDFView::_RenderCurrentPage()
{
    delete fRenderedBitmap;
    fRenderedBitmap = nullptr;

    if (fDocument && fDocument->IsValid() && fCurrentPage >= 0 && fCurrentPage < fDocument->PageCount()) {
        fRenderedBitmap = fDocument->RenderPageToBitmap(fCurrentPage, fZoom);
    }

    _UpdateScrollBars();
    Invalidate();
}

BRect
PDFView::_CalculatePageRect() const
{
    BRect bounds = Bounds();
    if (fRenderedBitmap == nullptr)
        return BRect(0, 0, 0, 0);

    float bmpW = fRenderedBitmap->Bounds().Width();
    float bmpH = fRenderedBitmap->Bounds().Height();

    float x = 20.0f;
    if (bounds.Width() > bmpW + 40.0f)
        x = bounds.left + (bounds.Width() - bmpW) / 2.0f;
    else
        x = 20.0f;

    float y = 20.0f;
    if (bounds.Height() > bmpH + 40.0f)
        y = bounds.top + (bounds.Height() - bmpH) / 2.0f;
    else
        y = 20.0f;

    return BRect(x, y, x + bmpW, y + bmpH);
}
