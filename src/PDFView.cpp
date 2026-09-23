#include "PDFView.h"

#include <Cursor.h>
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
      fToolMode(MODE_VIEW),
      fIsDragging(false),
      fLastMousePos(0, 0),
      fIsAnnotating(false),
      fDragStart(0, 0),
      fDragCurrent(0, 0)
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

    // Draw existing annotations overlay
    if (fDocument != nullptr) {
        const auto& annotations = fDocument->Annotations(fCurrentPage);
        for (const auto& ann : annotations) {
            BRect annRect(
                pageRect.left + ann.pageUnitBounds.left * pageRect.Width(),
                pageRect.top + ann.pageUnitBounds.top * pageRect.Height(),
                pageRect.left + ann.pageUnitBounds.right * pageRect.Width(),
                pageRect.top + ann.pageUnitBounds.bottom * pageRect.Height()
            );

            if (ann.type == ANNOTATION_HIGHLIGHT) {
                SetDrawingMode(B_OP_ALPHA);
                SetHighColor(255, 235, 59, 120);
                FillRect(annRect);
                SetDrawingMode(B_OP_COPY);
            } else if (ann.type == ANNOTATION_RECT) {
                SetHighColor(ann.color);
                SetPenSize(2.0f);
                StrokeRect(annRect);
                SetPenSize(1.0f);
            } else if (ann.type == ANNOTATION_TEXT) {
                SetHighColor(255, 245, 157);
                FillRect(annRect);
                SetHighColor(200, 180, 50);
                StrokeRect(annRect);

                SetHighColor(30, 30, 30);
                BFont noteFont;
                GetFont(&noteFont);
                noteFont.SetSize(11.0f);
                SetFont(&noteFont);
                DrawString(ann.text.String(), BPoint(annRect.left + 5.0f, annRect.top + 14.0f));
            }
        }
    }

    // Draw live overlay while dragging interactive annotation
    if (fIsAnnotating) {
        BRect selRect(
            std::min(fDragStart.x, fDragCurrent.x),
            std::min(fDragStart.y, fDragCurrent.y),
            std::max(fDragStart.x, fDragCurrent.x),
            std::max(fDragStart.y, fDragCurrent.y)
        );

        if (fToolMode == MODE_HIGHLIGHT) {
            SetDrawingMode(B_OP_ALPHA);
            SetHighColor(255, 235, 59, 100);
            FillRect(selRect);
            SetDrawingMode(B_OP_COPY);
            SetHighColor(210, 180, 20);
            StrokeRect(selRect);
        } else if (fToolMode == MODE_DRAW_RECT) {
            SetHighColor(220, 20, 60);
            SetPenSize(2.0f);
            StrokeRect(selRect);
            SetPenSize(1.0f);
        } else if (fToolMode == MODE_ANNOTATE_TEXT) {
            SetHighColor(255, 245, 157);
            FillRect(selRect);
            SetHighColor(200, 180, 50);
            StrokeRect(selRect);
        }
    }
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
        if (fToolMode == MODE_VIEW) {
            fIsDragging = true;
            fLastMousePos = where;
            SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
        } else if (fDocument != nullptr && fRenderedBitmap != nullptr) {
            BRect pageRect = _CalculatePageRect();
            if (pageRect.Contains(where)) {
                fIsAnnotating = true;
                fDragStart = where;
                fDragCurrent = where;
                SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
            }
        }
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
    } else if (fIsAnnotating && fRenderedBitmap != nullptr) {
        BRect pageRect = _CalculatePageRect();
        fDragCurrent.x = std::clamp(where.x, pageRect.left, pageRect.right);
        fDragCurrent.y = std::clamp(where.y, pageRect.top, pageRect.bottom);
        Invalidate();
    }
    BView::MouseMoved(where, transit, dragMessage);
}

void
PDFView::MouseUp(BPoint where)
{
    if (fIsDragging) {
        fIsDragging = false;
    } else if (fIsAnnotating) {
        fIsAnnotating = false;

        if (fDocument != nullptr && fRenderedBitmap != nullptr) {
            BRect pageRect = _CalculatePageRect();
            BRect selRect(
                std::min(fDragStart.x, fDragCurrent.x),
                std::min(fDragStart.y, fDragCurrent.y),
                std::max(fDragStart.x, fDragCurrent.x),
                std::max(fDragStart.y, fDragCurrent.y)
            );

            if (fToolMode == MODE_ANNOTATE_TEXT && selRect.Width() < 20.0f) {
                selRect.right = std::min(pageRect.right, selRect.left + 120.0f);
                selRect.bottom = std::min(pageRect.bottom, selRect.top + 40.0f);
            }

            if (selRect.Width() > 4.0f && selRect.Height() > 4.0f) {
                BRect unitBounds(
                    (selRect.left - pageRect.left) / pageRect.Width(),
                    (selRect.top - pageRect.top) / pageRect.Height(),
                    (selRect.right - pageRect.left) / pageRect.Width(),
                    (selRect.bottom - pageRect.top) / pageRect.Height()
                );

                Annotation ann;
                ann.pageUnitBounds = unitBounds;

                if (fToolMode == MODE_HIGHLIGHT) {
                    ann.type = ANNOTATION_HIGHLIGHT;
                    ann.color = rgb_color{255, 235, 59, 120};
                } else if (fToolMode == MODE_DRAW_RECT) {
                    ann.type = ANNOTATION_RECT;
                    ann.color = rgb_color{220, 20, 60, 255};
                } else if (fToolMode == MODE_ANNOTATE_TEXT) {
                    ann.type = ANNOTATION_TEXT;
                    ann.text = "Nota";
                    ann.color = rgb_color{255, 245, 157, 255};
                }

                fDocument->AddAnnotation(fCurrentPage, ann);
            }
        }
        Invalidate();
    }
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
PDFView::SetToolMode(ToolMode mode)
{
    fToolMode = mode;
}

void
PDFView::RotateCurrentPage(int32 degrees)
{
    if (fDocument != nullptr) {
        fDocument->RotatePage(fCurrentPage, degrees);
        _RenderCurrentPage();
    }
}

bool
PDFView::DeleteCurrentPage()
{
    if (fDocument != nullptr && fDocument->DeletePage(fCurrentPage)) {
        if (fCurrentPage >= fDocument->PageCount() && fCurrentPage > 0)
            fCurrentPage--;
        _RenderCurrentPage();
        return true;
    }
    return false;
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
