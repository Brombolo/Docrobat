#ifndef PDF_SECURITY_H
#define PDF_SECURITY_H

#include "PDFDocument.h"

#include <String.h>
#include <memory>

struct DocumentPermissions {
    bool    isEncrypted;
    bool    canPrint;
    bool    canModify;
    bool    canCopy;
    bool    canAnnotate;
    bool    canFillForms;
    BString encryptionAlgorithm;
};

class PDFSecurity {
public:
                                    PDFSecurity(std::shared_ptr<PDFDocument> doc = nullptr);
    virtual                         ~PDFSecurity();

    void                            SetDocument(std::shared_ptr<PDFDocument> doc);
    std::shared_ptr<PDFDocument>    Document() const { return fDocument; }

    bool                            IsLocked() const;
    bool                            Unlock(const char* password);

    DocumentPermissions             GetPermissions() const;
    bool                            SetPermissions(const DocumentPermissions& permissions, const char* ownerPassword);
    bool                            RemoveRestrictions(const char* ownerPassword);
    bool                            ProtectDocument(const char* userPassword, const char* ownerPassword,
                                        const DocumentPermissions& perms);

private:
    std::shared_ptr<PDFDocument>    fDocument;
    DocumentPermissions             fCurrentPermissions;
    BString                         fUserPassword;
    BString                         fOwnerPassword;
};

#endif // PDF_SECURITY_H
