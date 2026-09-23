#include "PDFFormManager.h"

#include <MenuItem.h>
#include <Node.h>
#include <String.h>

PDFFormManager::PDFFormManager(std::shared_ptr<PDFDocument> doc)
    : fDocument(doc)
{
}

PDFFormManager::~PDFFormManager()
{
    // Detach any lingering controls
    for (BView* ctrl : fActiveAttachedControls) {
        if (ctrl->Parent() != nullptr)
            ctrl->Parent()->RemoveChild(ctrl);
        delete ctrl;
    }
    fActiveAttachedControls.clear();
}

void
PDFFormManager::SetDocument(std::shared_ptr<PDFDocument> doc)
{
    fDocument = doc;
    fPageFields.clear();

    if (fDocument != nullptr && fDocument->IsValid()) {
        // Inizializza campi modulo per il documento (es. campi su prima pagina)
        FormField nameField;
        nameField.name = "Nome_Completo";
        nameField.type = FIELD_TEXT;
        nameField.pageUnitBounds = BRect(0.15f, 0.12f, 0.55f, 0.16f);
        nameField.value = "";
        nameField.readOnly = false;
        fPageFields[0].push_back(nameField);

        FormField signCheckbox;
        signCheckbox.name = "Accetto_Termini";
        signCheckbox.type = FIELD_CHECKBOX;
        signCheckbox.pageUnitBounds = BRect(0.15f, 0.18f, 0.40f, 0.22f);
        signCheckbox.value = "false";
        signCheckbox.readOnly = false;
        fPageFields[0].push_back(signCheckbox);

        FormField roleChoice;
        roleChoice.name = "Ruolo_Utente";
        roleChoice.type = FIELD_CHOICE;
        roleChoice.pageUnitBounds = BRect(0.15f, 0.24f, 0.50f, 0.28f);
        roleChoice.value = "Sviluppatore";
        roleChoice.options.push_back("Amministratore");
        roleChoice.options.push_back("Sviluppatore");
        roleChoice.options.push_back("Revisore");
        roleChoice.readOnly = false;
        fPageFields[0].push_back(roleChoice);
    }
}

std::vector<FormField>
PDFFormManager::GetFields(int32 pageIndex)
{
    if (fPageFields.find(pageIndex) != fPageFields.end())
        return fPageFields[pageIndex];
    return std::vector<FormField>();
}

bool
PDFFormManager::SetFieldValue(int32 pageIndex, const char* fieldName, const char* value)
{
    if (fieldName == nullptr || value == nullptr)
        return false;

    auto it = fPageFields.find(pageIndex);
    if (it != fPageFields.end()) {
        for (auto& field : it->second) {
            if (field.name == fieldName) {
                field.value = value;
                return true;
            }
        }
    }
    return false;
}

bool
PDFFormManager::AddFormField(int32 pageIndex, const FormField& field)
{
    fPageFields[pageIndex].push_back(field);
    return true;
}

void
PDFFormManager::AttachControlsToView(BView* targetView, int32 pageIndex, const BRect& pageRect)
{
    if (targetView == nullptr || pageRect.Width() <= 0 || pageRect.Height() <= 0)
        return;

    DetachControlsFromView(targetView);

    auto it = fPageFields.find(pageIndex);
    if (it == fPageFields.end())
        return;

    for (const auto& field : it->second) {
        BRect ctrlRect(
            pageRect.left + field.pageUnitBounds.left * pageRect.Width(),
            pageRect.top + field.pageUnitBounds.top * pageRect.Height(),
            pageRect.left + field.pageUnitBounds.right * pageRect.Width(),
            pageRect.top + field.pageUnitBounds.bottom * pageRect.Height()
        );

        switch (field.type) {
            case FIELD_TEXT:
            {
                BTextControl* textCtrl = new BTextControl(
                    ctrlRect,
                    field.name.String(),
                    "",
                    field.value.String(),
                    new BMessage('FRMC')
                );
                targetView->AddChild(textCtrl);
                fActiveAttachedControls.push_back(textCtrl);
                break;
            }

            case FIELD_CHECKBOX:
            {
                BCheckBox* checkCtrl = new BCheckBox(
                    ctrlRect,
                    field.name.String(),
                    field.name.String(),
                    new BMessage('FRMC')
                );
                if (field.value == "true" || field.value == "1")
                    checkCtrl->SetValue(B_CONTROL_ON);
                targetView->AddChild(checkCtrl);
                fActiveAttachedControls.push_back(checkCtrl);
                break;
            }

            case FIELD_CHOICE:
            {
                BPopUpMenu* popMenu = new BPopUpMenu("Scelta");
                for (const auto& opt : field.options) {
                    BMenuItem* item = new BMenuItem(opt.String(), new BMessage('FRMC'));
                    if (opt == field.value)
                        item->SetMarked(true);
                    popMenu->AddItem(item);
                }

                BMenuField* menuField = new BMenuField(
                    ctrlRect,
                    field.name.String(),
                    "",
                    popMenu
                );
                targetView->AddChild(menuField);
                fActiveAttachedControls.push_back(menuField);
                break;
            }

            case FIELD_SIGNATURE:
            default:
                break;
        }
    }
}

void
PDFFormManager::DetachControlsFromView(BView* targetView)
{
    if (targetView == nullptr)
        return;

    for (BView* ctrl : fActiveAttachedControls) {
        if (ctrl != nullptr && ctrl->Parent() == targetView) {
            targetView->RemoveChild(ctrl);
            delete ctrl;
        }
    }
    fActiveAttachedControls.clear();
}

int32
PDFFormManager::TotalFieldsCount() const
{
    int32 count = 0;
    for (const auto& pair : fPageFields) {
        count += static_cast<int32>(pair.second.size());
    }
    return count;
}
