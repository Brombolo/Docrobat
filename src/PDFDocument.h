#ifndef PDF_DOCUMENT_H
#define PDF_DOCUMENT_H

#include <Bitmap.h>
#include <Rect.h>
#include <Size.h>
#include <SupportDefs.h>

#include <memory>
#include <string>

namespace poppler {
    class document;
    class page;
}

class PDFDocument {
public:
                                PDFDocument();
    virtual                     ~PDFDocument();

    bool                        LoadFromFile(const char* filePath);
    bool                        IsValid() const;
    bool                        IsLocked() const;

    int32                       PageCount() const;
    BSize                       PageSize(int32 pageIndex) const;

    BBitmap*                    RenderPageToBitmap(int32 pageIndex, float scale = 1.0f);

    const char*                 FilePath() const { return fFilePath.c_str(); }

private:
    poppler::document*          fDocument;
    std::string                 fFilePath;
    int32                       fPageCount;
};

#endif // PDF_DOCUMENT_H
