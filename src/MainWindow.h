#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "Annotation.h"
#include "PDFObjectEditor.h"
#include "PDFFormManager.h"
#include "PDFSecurity.h"

#include <Window.h>
#include <FilePanel.h>
#include <Entry.h>

class BMenuBar;
class BGroupView;
class BButton;
class BStringView;
class BScrollView;
class BTabView;
class BListView;
class BOutlineListView;
class PDFView;

// Haiku 4-character message constants
enum {
    MSG_FILE_OPEN           = 'DOCO',
    MSG_FILE_CLOSE          = 'DOCC',
    MSG_FILE_SAVE           = 'DOCS',
    MSG_FILE_SAVE_AS        = 'DOCA',
    MSG_PAGE_PREV           = 'PGUP',
    MSG_PAGE_NEXT           = 'PGDN',
    MSG_ZOOM                = 'ZOOM',
    MSG_ZOOM_IN             = 'ZMIN',
    MSG_ZOOM_OUT            = 'ZOUT',
    MSG_ZOOM_100            = 'Z100',
    MSG_ZOOM_FIT            = 'ZFPG',
    MSG_ROTATE_LEFT         = 'ROTL',
    MSG_ROTATE_RIGHT        = 'ROTR',
    MSG_DELETE_PAGE         = 'DELP',
    MSG_TOOL_VIEW           = 'TMOV',
    MSG_TOOL_HIGHLIGHT      = 'THIL',
    MSG_TOOL_TEXT           = 'TTXT',
    MSG_TOOL_RECT           = 'TREC',
    MSG_TOGGLE_SIDEBAR      = 'SIDE',
    MSG_SELECT_THUMBNAIL    = 'THMB',
    MSG_SELECT_BOOKMARK     = 'BKMK',
    MSG_SECURITY_UNLOCK     = 'SECU',
    MSG_SECURITY_PROTECT    = 'SECP',
    MSG_APPLY_FORM          = 'FRMA'
};

class MainWindow : public BWindow {
public:
                            MainWindow();
    virtual                 ~MainWindow();

    virtual void            MessageReceived(BMessage* message) override;
    virtual bool            QuitRequested() override;

    void                    OpenFile(const entry_ref& ref);
    void                    OpenFile(const char* path);
    void                    SaveFile(const char* path);

private:
    void                    _BuildLayout();
    void                    _BuildSidebar();
    void                    _UpdateControls();
    void                    _UpdateToolButtons();
    void                    _UpdateSidebarData();
    void                    _ToggleSidebar();

    BMenuBar*               fMenuBar;
    BGroupView*             fNavToolBar;
    BGroupView*             fEditToolBar;
    PDFView*                fPDFView;
    BScrollView*            fScrollView;
    BFilePanel*             fOpenPanel;
    BFilePanel*             fSavePanel;

    // Navigation Controls
    BButton*                fOpenButton;
    BButton*                fPrevButton;
    BButton*                fNextButton;
    BStringView*            fPageInfoView;
    BButton*                fZoomOutButton;
    BStringView*            fZoomInfoView;
    BButton*                fZoomInButton;
    BButton*                fZoomFitButton;
    BButton*                fZoomResetButton;
    BButton*                fSidebarToggleBtn;

    // Editing & Tool Controls
    BButton*                fToolViewBtn;
    BButton*                fToolHighlightBtn;
    BButton*                fToolTextBtn;
    BButton*                fToolRectBtn;
    BButton*                fRotateLeftBtn;
    BButton*                fRotateRightBtn;
    BButton*                fDeletePageBtn;
    BButton*                fSaveButton;

    // Sidebar TabView and Tabs
    BTabView*               fSidebarTabView;
    bool                    fSidebarVisible;

    // Tab 1: Miniature
    BListView*              fThumbnailList;
    BScrollView*            fThumbnailScroll;

    // Tab 2: Segnalibri / Struttura
    BOutlineListView*       fBookmarksTree;
    BScrollView*            fBookmarksScroll;

    // Tab 3: Moduli e Campi
    BListView*              fFormFieldsList;
    BScrollView*            fFormFieldsScroll;
    BButton*                fApplyFormBtn;

    // Tab 4: Proprietà e Sicurezza
    BStringView*            fDocTitleView;
    BStringView*            fDocPagesView;
    BStringView*            fSecurityStatusView;
    BStringView*            fPermissionsView;
    BButton*                fUnlockBtn;
    BButton*                fProtectBtn;

    // Modular managers
    std::unique_ptr<PDFObjectEditor> fObjectEditor;
    std::unique_ptr<PDFFormManager>  fFormManager;
    std::unique_ptr<PDFSecurity>     fSecurity;
};

#endif // MAIN_WINDOW_H
