#ifndef PDF_OBJECT_EDITOR_H
#define PDF_OBJECT_EDITOR_H

#include "PDFDocument.h"

#include <Bitmap.h>
#include <Point.h>
#include <Rect.h>
#include <String.h>

#include <memory>
#include <vector>

struct PDFTextBlock {
    BRect   bounds;
    BString text;
};

struct PDFImageObject {
    int32   id;
    BRect   pageBounds;
    int32   width;
    int32   height;
    BString format;
};

class PDFObjectEditor {
public:
                                    PDFObjectEditor(std::shared_ptr<PDFDocument> document = nullptr);
    virtual                         ~PDFObjectEditor();

    void                            SetDocument(std::shared_ptr<PDFDocument> document);
    std::shared_ptr<PDFDocument>    Document() const { return fDocument; }

    // Text object inspection & extraction
    std::vector<PDFTextBlock>       ExtractTextBlocks(int32 pageIndex);
    bool                            AddText(int32 pageIndex, const BPoint& where, const char* text, float fontSize = 12.0f);

    // Image object inspection & extraction
    std::vector<PDFImageObject>     FindImages(int32 pageIndex);
    BBitmap*                        ExtractImage(int32 pageIndex, int32 imageIndex);
    bool                            ReplaceImage(int32 pageIndex, int32 imageIndex, const BBitmap* replacement);

    // Object removal
    bool                            DeleteObject(int32 pageIndex, const BRect& bounds);

private:
    std::shared_ptr<PDFDocument>    fDocument;
    std::vector<PDFImageObject>     fCachedImages;
    int32                           fCachedPageIndex;
};

#endif // PDF_OBJECT_EDITOR_H
