#ifndef PDF_FORM_MANAGER_H
#define PDF_FORM_MANAGER_H

#include "PDFDocument.h"

#include <Rect.h>
#include <String.h>
#include <View.h>
#include <TextControl.h>
#include <CheckBox.h>
#include <MenuField.h>
#include <PopUpMenu.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

enum FormFieldType {
    FIELD_TEXT = 0,
    FIELD_CHECKBOX,
    FIELD_CHOICE,
    FIELD_SIGNATURE
};

struct FormField {
    BString                 name;
    FormFieldType           type;
    BRect                   pageUnitBounds; // Normalizzate [0, 1] rispetto alla pagina
    BString                 value;
    std::vector<BString>    options;        // Opzioni per popup/combobox
    bool                    readOnly;
};

class PDFFormManager {
public:
                                    PDFFormManager(std::shared_ptr<PDFDocument> doc = nullptr);
    virtual                         ~PDFFormManager();

    void                            SetDocument(std::shared_ptr<PDFDocument> doc);
    std::shared_ptr<PDFDocument>    Document() const { return fDocument; }

    std::vector<FormField>          GetFields(int32 pageIndex);
    bool                            SetFieldValue(int32 pageIndex, const char* fieldName, const char* value);
    bool                            AddFormField(int32 pageIndex, const FormField& field);

    // Binding nativo controlli Haiku posizionati sopra la pagina
    void                            AttachControlsToView(BView* targetView, int32 pageIndex, const BRect& pageRect);
    void                            DetachControlsFromView(BView* targetView);

    int32                           TotalFieldsCount() const;

private:
    std::shared_ptr<PDFDocument>    fDocument;
    std::map<int32, std::vector<FormField>> fPageFields;
    std::vector<BView*>             fActiveAttachedControls;
};

#endif // PDF_FORM_MANAGER_H
