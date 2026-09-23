#ifndef PDF_DOCUMENT_H
#define PDF_DOCUMENT_H

#include "Annotation.h"

#include <Bitmap.h>
#include <Rect.h>
#include <Size.h>
#include <SupportDefs.h>

#include <memory>
#include <string>
#include <vector>

namespace poppler {
    class document;
    class page;
}

struct PageInfo {
    int32                   originalIndex;
    int32                   rotation; // 0, 90, 180, 270 gradi
    std::vector<Annotation> annotations;
};

class PDFDocument {
public:
                                PDFDocument();
    virtual                     ~PDFDocument();

    bool                        LoadFromFile(const char* filePath);
    bool                        IsValid() const;
    bool                        IsLocked() const;

    int32                       PageCount() const;
    BSize                       PageSize(int32 pageIndex) const;
    int32                       PageRotation(int32 pageIndex) const;

    // Structural manipulation
    void                        RotatePage(int32 pageIndex, int32 degrees);
    bool                        DeletePage(int32 pageIndex);
    bool                        ReorderPage(int32 oldIndex, int32 newIndex);

    // Save
    bool                        SaveAs(const char* filePath);

    // Annotations
    std::vector<Annotation>&    Annotations(int32 pageIndex);
    const std::vector<Annotation>& Annotations(int32 pageIndex) const;
    void                        AddAnnotation(int32 pageIndex, const Annotation& annotation);

    // Rendering
    BBitmap*                    RenderPageToBitmap(int32 pageIndex, float scale = 1.0f);

    const char*                 FilePath() const { return fFilePath.c_str(); }

private:
    poppler::document*          fDocument;
    std::string                 fFilePath;
    std::vector<PageInfo>       fPages;
};

#endif // PDF_DOCUMENT_H
