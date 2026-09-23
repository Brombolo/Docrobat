#ifndef PDF_VIEW_H
#define PDF_VIEW_H

#include <View.h>
#include <Bitmap.h>
#include <Point.h>
#include <Rect.h>

class PDFView : public BView {
public:
                            PDFView(const char* name);
    virtual                 ~PDFView();

    virtual void            AttachedToWindow() override;
    virtual void            Draw(BRect updateRect) override;
    virtual void            FrameResized(float newWidth, float newHeight) override;
    virtual void            MouseDown(BPoint where) override;
    virtual void            MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage) override;
    virtual void            MouseUp(BPoint where) override;

    void                    SetBitmap(BBitmap* bitmap);
    BBitmap*                Bitmap() const { return fRenderedBitmap; }

    void                    SetZoom(float zoom);
    float                   Zoom() const { return fZoom; }

    int32                   CurrentPage() const { return fCurrentPage; }
    int32                   TotalPages() const { return fTotalPages; }
    void                    SetPageInfo(int32 current, int32 total);

    bool                    LoadDocument(const char* filePath);

    void                    NextPage();
    void                    PreviousPage();

private:
    void                    _UpdateScrollBars();
    void                    _RenderCurrentPage();

    BBitmap*                fRenderedBitmap;
    float                   fZoom;
    int32                   fCurrentPage;
    int32                   fTotalPages;
    bool                    fIsDragging;
    BPoint                  fLastMousePos;
    char*                   fCurrentFilePath;
};

#endif // PDF_VIEW_H
