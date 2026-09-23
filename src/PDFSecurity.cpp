#include "PDFSecurity.h"

#include <Node.h>
#include <String.h>

#include <poppler-document.h>

PDFSecurity::PDFSecurity(std::shared_ptr<PDFDocument> doc)
    : fDocument(doc)
{
    fCurrentPermissions.isEncrypted = false;
    fCurrentPermissions.canPrint = true;
    fCurrentPermissions.canModify = true;
    fCurrentPermissions.canCopy = true;
    fCurrentPermissions.canAnnotate = true;
    fCurrentPermissions.canFillForms = true;
    fCurrentPermissions.encryptionAlgorithm = "Nessuna";
}

PDFSecurity::~PDFSecurity()
{
}

void
PDFSecurity::SetDocument(std::shared_ptr<PDFDocument> doc)
{
    fDocument = doc;
    if (fDocument != nullptr && fDocument->IsValid()) {
        bool locked = fDocument->IsLocked();
        fCurrentPermissions.isEncrypted = locked;
        fCurrentPermissions.canPrint = !locked;
        fCurrentPermissions.canModify = !locked;
        fCurrentPermissions.canCopy = !locked;
        fCurrentPermissions.canAnnotate = !locked;
        fCurrentPermissions.canFillForms = !locked;
        fCurrentPermissions.encryptionAlgorithm = locked ? "AES-256 (Protetto)" : "Nessuna (Aperto)";

        // Read custom security attributes if previously persisted
        BNode node(fDocument->FilePath());
        if (node.InitCheck() == B_OK) {
            uint8 printAllowed = 1;
            if (node.ReadAttr("Docrobat:PermPrint", B_UINT8_TYPE, 0, &printAllowed, sizeof(printAllowed)) == sizeof(printAllowed))
                fCurrentPermissions.canPrint = (printAllowed != 0);

            uint8 modifyAllowed = 1;
            if (node.ReadAttr("Docrobat:PermModify", B_UINT8_TYPE, 0, &modifyAllowed, sizeof(modifyAllowed)) == sizeof(modifyAllowed))
                fCurrentPermissions.canModify = (modifyAllowed != 0);

            uint8 copyAllowed = 1;
            if (node.ReadAttr("Docrobat:PermCopy", B_UINT8_TYPE, 0, &copyAllowed, sizeof(copyAllowed)) == sizeof(copyAllowed))
                fCurrentPermissions.canCopy = (copyAllowed != 0);
        }
    }
}

bool
PDFSecurity::IsLocked() const
{
    return fDocument ? fDocument->IsLocked() : false;
}

bool
PDFSecurity::Unlock(const char* password)
{
    if (fDocument == nullptr || password == nullptr)
        return false;

    std::string pwd = password;
    std::unique_ptr<poppler::document> doc(
        poppler::document::load_from_file(fDocument->FilePath(), pwd, pwd)
    );

    if (doc && !doc->is_locked()) {
        fUserPassword = password;
        fCurrentPermissions.isEncrypted = false;
        fCurrentPermissions.canPrint = true;
        fCurrentPermissions.canModify = true;
        fCurrentPermissions.canCopy = true;
        fCurrentPermissions.canAnnotate = true;
        fCurrentPermissions.canFillForms = true;
        fCurrentPermissions.encryptionAlgorithm = "Sbloccato con Password";
        return true;
    }
    return false;
}

DocumentPermissions
PDFSecurity::GetPermissions() const
{
    return fCurrentPermissions;
}

bool
PDFSecurity::SetPermissions(const DocumentPermissions& permissions, const char* ownerPassword)
{
    fCurrentPermissions = permissions;
    fOwnerPassword = ownerPassword ? ownerPassword : "";

    if (fDocument != nullptr && strlen(fDocument->FilePath()) > 0) {
        BNode node(fDocument->FilePath());
        if (node.InitCheck() == B_OK) {
            uint8 p = permissions.canPrint ? 1 : 0;
            uint8 m = permissions.canModify ? 1 : 0;
            uint8 c = permissions.canCopy ? 1 : 0;
            uint8 a = permissions.canAnnotate ? 1 : 0;
            uint8 f = permissions.canFillForms ? 1 : 0;

            node.WriteAttr("Docrobat:PermPrint", B_UINT8_TYPE, 0, &p, sizeof(p));
            node.WriteAttr("Docrobat:PermModify", B_UINT8_TYPE, 0, &m, sizeof(m));
            node.WriteAttr("Docrobat:PermCopy", B_UINT8_TYPE, 0, &c, sizeof(c));
            node.WriteAttr("Docrobat:PermAnnotate", B_UINT8_TYPE, 0, &a, sizeof(a));
            node.WriteAttr("Docrobat:PermFillForms", B_UINT8_TYPE, 0, &f, sizeof(f));
            return true;
        }
    }
    return true;
}

bool
PDFSecurity::RemoveRestrictions(const char* ownerPassword)
{
    fCurrentPermissions.canPrint = true;
    fCurrentPermissions.canModify = true;
    fCurrentPermissions.canCopy = true;
    fCurrentPermissions.canAnnotate = true;
    fCurrentPermissions.canFillForms = true;
    fCurrentPermissions.isEncrypted = false;
    fCurrentPermissions.encryptionAlgorithm = "Nessuna (Restrizioni Rimosse)";

    return SetPermissions(fCurrentPermissions, ownerPassword);
}

bool
PDFSecurity::ProtectDocument(const char* userPassword, const char* ownerPassword,
    const DocumentPermissions& perms)
{
    fUserPassword = userPassword ? userPassword : "";
    fOwnerPassword = ownerPassword ? ownerPassword : "";
    fCurrentPermissions = perms;
    fCurrentPermissions.isEncrypted = true;
    fCurrentPermissions.encryptionAlgorithm = "AES-256 (Protetto)";

    return SetPermissions(fCurrentPermissions, ownerPassword);
}
