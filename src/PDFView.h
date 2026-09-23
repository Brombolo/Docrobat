#ifndef PDF_VIEW_H
#define PDF_VIEW_H

#include "PDFDocument.h"

#include <View.h>
#include <Bitmap.h>
#include <Point.h>
#include <Rect.h>

#include <memory>

class PDFView : public BView {
public:
                                PDFView(const char* name);
    virtual                     ~PDFView();

    virtual void                AttachedToWindow() override;
    virtual void                Draw(BRect updateRect) override;
    virtual void                FrameResized(float newWidth, float newHeight) override;
    virtual void                MouseDown(BPoint where) override;
    virtual void                MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage) override;
    virtual void                MouseUp(BPoint where) override;
    virtual void                KeyDown(const char* bytes, int32 numBytes) override;

    void                        SetDocument(std::shared_ptr<PDFDocument> document);
    std::shared_ptr<PDFDocument> Document() const { return fDocument; }

    bool                        LoadDocument(const char* filePath);

    void                        SetCurrentPage(int32 pageIndex);
    int32                       CurrentPage() const { return fCurrentPage; }
    int32                       PageCount() const;

    void                        SetZoom(float zoom);
    float                       Zoom() const { return fZoom; }

    void                        NextPage();
    void                        PreviousPage();
    void                        FirstPage();
    void                        LastPage();
    void                        RefreshPage();

    BBitmap*                    Bitmap() const { return fRenderedBitmap; }

private:
    void                        _UpdateScrollBars();
    void                        _RenderCurrentPage();
    BRect                       _CalculatePageRect() const;

    std::shared_ptr<PDFDocument> fDocument;
    int32                       fCurrentPage;
    float                       fZoom;
    BBitmap*                    fRenderedBitmap;

    bool                        fIsDragging;
    BPoint                      fLastMousePos;
};

#endif // PDF_VIEW_H
