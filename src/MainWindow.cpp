#include "MainWindow.h"
#include "PDFView.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <GroupView.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <OutlineListView.h>
#include <Path.h>
#include <ScrollView.h>
#include <SplitView.h>
#include <String.h>
#include <StringItem.h>
#include <StringView.h>
#include <TabView.h>

#include <cmath>

MainWindow::MainWindow()
    : BWindow(BRect(60, 60, 1080, 820), "Docrobat - PDF Editor Professionale", B_TITLED_WINDOW,
        B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS),
      fMenuBar(nullptr),
      fNavToolBar(nullptr),
      fEditToolBar(nullptr),
      fPDFView(nullptr),
      fScrollView(nullptr),
      fOpenPanel(nullptr),
      fSavePanel(nullptr),
      fOpenButton(nullptr),
      fPrevButton(nullptr),
      fNextButton(nullptr),
      fPageInfoView(nullptr),
      fZoomOutButton(nullptr),
      fZoomInfoView(nullptr),
      fZoomInButton(nullptr),
      fZoomFitButton(nullptr),
      fZoomResetButton(nullptr),
      fSidebarToggleBtn(nullptr),
      fToolViewBtn(nullptr),
      fToolHighlightBtn(nullptr),
      fToolTextBtn(nullptr),
      fToolRectBtn(nullptr),
      fRotateLeftBtn(nullptr),
      fRotateRightBtn(nullptr),
      fDeletePageBtn(nullptr),
      fSaveButton(nullptr),
      fSidebarTabView(nullptr),
      fSidebarVisible(true),
      fThumbnailList(nullptr),
      fThumbnailScroll(nullptr),
      fBookmarksTree(nullptr),
      fBookmarksScroll(nullptr),
      fFormFieldsList(nullptr),
      fFormFieldsScroll(nullptr),
      fApplyFormBtn(nullptr),
      fDocTitleView(nullptr),
      fDocPagesView(nullptr),
      fSecurityStatusView(nullptr),
      fPermissionsView(nullptr),
      fUnlockBtn(nullptr),
      fProtectBtn(nullptr),
      fObjectEditor(std::make_unique<PDFObjectEditor>()),
      fFormManager(std::make_unique<PDFFormManager>()),
      fSecurity(std::make_unique<PDFSecurity>())
{
    _BuildLayout();

    fOpenPanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), nullptr, B_FILE_NODE, false);
    fSavePanel = new BFilePanel(B_SAVE_PANEL, new BMessenger(this), nullptr, B_FILE_NODE, false);

    _UpdateControls();
    _UpdateToolButtons();
    _UpdateSidebarData();
}

MainWindow::~MainWindow()
{
    delete fOpenPanel;
    delete fSavePanel;
}

void
MainWindow::_BuildSidebar()
{
    fSidebarTabView = new BTabView("sidebarTabView", B_WIDTH_FROM_LABEL);

    // --- Tab 1: Pagine e Miniature ---
    BGroupView* tab1View = new BGroupView(B_VERTICAL);
    fThumbnailList = new BListView("thumbList", B_SINGLE_SELECTION_LIST);
    fThumbnailList->SetSelectionMessage(new BMessage(MSG_SELECT_THUMBNAIL));
    fThumbnailScroll = new BScrollView("thumbScroll", fThumbnailList, 0, false, true, B_PLAIN_BORDER);

    BLayoutBuilder::Group<>(tab1View, B_VERTICAL, 0)
        .SetInsets(B_USE_SMALL_INSETS)
        .Add(fThumbnailScroll)
        .End();

    BTab* tab1 = new BTab();
    fSidebarTabView->AddTab(tab1View, tab1);
    tab1->SetLabel("Pagine");

    // --- Tab 2: Struttura / Segnalibri (Bookmarks) ---
    BGroupView* tab2View = new BGroupView(B_VERTICAL);
    fBookmarksTree = new BOutlineListView("bookmarksTree", B_SINGLE_SELECTION_LIST);
    fBookmarksTree->SetSelectionMessage(new BMessage(MSG_SELECT_BOOKMARK));
    fBookmarksScroll = new BScrollView("bookmarksScroll", fBookmarksTree, 0, false, true, B_PLAIN_BORDER);

    BLayoutBuilder::Group<>(tab2View, B_VERTICAL, 0)
        .SetInsets(B_USE_SMALL_INSETS)
        .Add(fBookmarksScroll)
        .End();

    BTab* tab2 = new BTab();
    fSidebarTabView->AddTab(tab2View, tab2);
    tab2->SetLabel("Segnalibri");

    // --- Tab 3: Moduli e Campi (AcroForms) ---
    BGroupView* tab3View = new BGroupView(B_VERTICAL, B_USE_SMALL_SPACING);
    fFormFieldsList = new BListView("formFieldsList");
    fFormFieldsScroll = new BScrollView("formFieldsScroll", fFormFieldsList, 0, false, true, B_PLAIN_BORDER);
    fApplyFormBtn = new BButton("applyFormBtn", "Compila Campi su Pagina", new BMessage(MSG_APPLY_FORM));

    BLayoutBuilder::Group<>(tab3View, B_VERTICAL, B_USE_SMALL_SPACING)
        .SetInsets(B_USE_SMALL_INSETS)
        .Add(fFormFieldsScroll)
        .Add(fApplyFormBtn)
        .End();

    BTab* tab3 = new BTab();
    fSidebarTabView->AddTab(tab3View, tab3);
    tab3->SetLabel("Moduli");

    // --- Tab 4: Proprietà e Sicurezza Documento ---
    BGroupView* tab4View = new BGroupView(B_VERTICAL, B_USE_SMALL_SPACING);
    fDocTitleView = new BStringView("docTitleView", "Documento: Nessuno");
    fDocPagesView = new BStringView("docPagesView", "Totale pagine: 0");
    fSecurityStatusView = new BStringView("securityStatusView", "Sicurezza: Nessuna restrizione");
    fPermissionsView = new BStringView("permissionsView", "Permessi: Tutti abilitati");

    fUnlockBtn = new BButton("unlockBtn", "🔓 Sblocca con Password", new BMessage(MSG_SECURITY_UNLOCK));
    fProtectBtn = new BButton("protectBtn", "🔒 Proteggi Documento", new BMessage(MSG_SECURITY_PROTECT));

    BLayoutBuilder::Group<>(tab4View, B_VERTICAL, B_USE_SMALL_SPACING)
        .SetInsets(B_USE_DEFAULT_INSETS)
        .Add(fDocTitleView)
        .Add(fDocPagesView)
        .AddStrut(10.0f)
        .Add(fSecurityStatusView)
        .Add(fPermissionsView)
        .AddStrut(10.0f)
        .Add(fUnlockBtn)
        .Add(fProtectBtn)
        .AddGlue()
        .End();

    BTab* tab4 = new BTab();
    fSidebarTabView->AddTab(tab4View, tab4);
    tab4->SetLabel("Sicurezza");
}

void
MainWindow::_BuildLayout()
{
    // 1. Menu Bar
    fMenuBar = new BMenuBar("mainMenuBar");

    BMenu* fileMenu = new BMenu("File");
    fileMenu->AddItem(new BMenuItem("Apri...", new BMessage(MSG_FILE_OPEN), 'O'));
    fileMenu->AddItem(new BMenuItem("Salva", new BMessage(MSG_FILE_SAVE), 'S'));
    fileMenu->AddItem(new BMenuItem("Salva come...", new BMessage(MSG_FILE_SAVE_AS)));
    fileMenu->AddSeparatorItem();
    fileMenu->AddItem(new BMenuItem("Chiudi", new BMessage(MSG_FILE_CLOSE), 'W'));
    fileMenu->AddSeparatorItem();
    fileMenu->AddItem(new BMenuItem("Informazioni su Docrobat", new BMessage(B_ABOUT_REQUESTED)));
    fileMenu->AddSeparatorItem();
    fileMenu->AddItem(new BMenuItem("Esci", new BMessage(B_QUIT_REQUESTED), 'Q'));
    fMenuBar->AddItem(fileMenu);

    BMenu* editMenu = new BMenu("Modifica");
    editMenu->AddItem(new BMenuItem("Ruota a sinistra (90°)", new BMessage(MSG_ROTATE_LEFT), 'L'));
    editMenu->AddItem(new BMenuItem("Ruota a destra (90°)", new BMessage(MSG_ROTATE_RIGHT), 'R'));
    editMenu->AddSeparatorItem();
    editMenu->AddItem(new BMenuItem("Elimina pagina", new BMessage(MSG_DELETE_PAGE)));
    fMenuBar->AddItem(editMenu);

    BMenu* viewMenu = new BMenu("Visualizza");
    viewMenu->AddItem(new BMenuItem("Barra laterale", new BMessage(MSG_TOGGLE_SIDEBAR), 'B'));
    viewMenu->AddSeparatorItem();
    viewMenu->AddItem(new BMenuItem("Ingrandisci", new BMessage(MSG_ZOOM_IN), '+'));
    viewMenu->AddItem(new BMenuItem("Rimpicciolisci", new BMessage(MSG_ZOOM_OUT), '-'));
    viewMenu->AddItem(new BMenuItem("Dimensione reale (100%)", new BMessage(MSG_ZOOM_100), '0'));
    viewMenu->AddItem(new BMenuItem("Adatta a pagina", new BMessage(MSG_ZOOM_FIT)));
    fMenuBar->AddItem(viewMenu);

    BMenu* goMenu = new BMenu("Vai");
    goMenu->AddItem(new BMenuItem("Pagina precedente", new BMessage(MSG_PAGE_PREV), B_PAGE_UP));
    goMenu->AddItem(new BMenuItem("Pagina successiva", new BMessage(MSG_PAGE_NEXT), B_PAGE_DOWN));
    fMenuBar->AddItem(goMenu);

    // 2. Navigation & Zoom Toolbar (Row 1)
    fNavToolBar = new BGroupView(B_HORIZONTAL, B_USE_SMALL_SPACING);
    fNavToolBar->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

    fSidebarToggleBtn = new BButton("sidebarToggleBtn", "📑 Pannello", new BMessage(MSG_TOGGLE_SIDEBAR));
    fOpenButton = new BButton("openButton", "Apri...", new BMessage(MSG_FILE_OPEN));
    fPrevButton = new BButton("prevButton", "◀ Prec.", new BMessage(MSG_PAGE_PREV));
    fNextButton = new BButton("nextButton", "Succ. ▶", new BMessage(MSG_PAGE_NEXT));
    fPageInfoView = new BStringView("pageInfoView", "Nessun documento");

    fZoomOutButton = new BButton("zoomOutButton", "−", new BMessage(MSG_ZOOM_OUT));
    fZoomInfoView = new BStringView("zoomInfoView", "100%");
    fZoomInButton = new BButton("zoomInButton", "+", new BMessage(MSG_ZOOM_IN));
    fZoomResetButton = new BButton("zoomResetButton", "100%", new BMessage(MSG_ZOOM_100));
    fZoomFitButton = new BButton("zoomFitButton", "Adatta", new BMessage(MSG_ZOOM_FIT));

    BLayoutBuilder::Group<>(fNavToolBar, B_HORIZONTAL, B_USE_SMALL_SPACING)
        .SetInsets(B_USE_SMALL_INSETS)
        .Add(fSidebarToggleBtn)
        .AddStrut(6.0f)
        .Add(fOpenButton)
        .AddStrut(10.0f)
        .Add(fPrevButton)
        .Add(fNextButton)
        .AddStrut(6.0f)
        .Add(fPageInfoView)
        .AddGlue()
        .Add(fZoomOutButton)
        .Add(fZoomInfoView)
        .Add(fZoomInButton)
        .Add(fZoomResetButton)
        .Add(fZoomFitButton)
        .End();

    // 3. Edit & Annotation Tools Toolbar (Row 2)
    fEditToolBar = new BGroupView(B_HORIZONTAL, B_USE_SMALL_SPACING);
    fEditToolBar->SetViewColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_1_TINT));

    BStringView* toolLabel = new BStringView("toolLabel", "Strumenti:");
    fToolViewBtn = new BButton("toolViewBtn", "✋ Vista", new BMessage(MSG_TOOL_VIEW));
    fToolHighlightBtn = new BButton("toolHighlightBtn", "🖊 Evidenzia", new BMessage(MSG_TOOL_HIGHLIGHT));
    fToolTextBtn = new BButton("toolTextBtn", "📝 Nota", new BMessage(MSG_TOOL_TEXT));
    fToolRectBtn = new BButton("toolRectBtn", "▭ Rettangolo", new BMessage(MSG_TOOL_RECT));

    BStringView* pageLabel = new BStringView("pageLabel", "Pagina:");
    fRotateLeftBtn = new BButton("rotateLeftBtn", "↺ Ruota SX", new BMessage(MSG_ROTATE_LEFT));
    fRotateRightBtn = new BButton("rotateRightBtn", "↻ Ruota DX", new BMessage(MSG_ROTATE_RIGHT));
    fDeletePageBtn = new BButton("deletePageBtn", "🗑 Elimina", new BMessage(MSG_DELETE_PAGE));

    fSaveButton = new BButton("saveButton", "💾 Salva", new BMessage(MSG_FILE_SAVE));

    BLayoutBuilder::Group<>(fEditToolBar, B_HORIZONTAL, B_USE_SMALL_SPACING)
        .SetInsets(B_USE_SMALL_INSETS)
        .Add(toolLabel)
        .Add(fToolViewBtn)
        .Add(fToolHighlightBtn)
        .Add(fToolTextBtn)
        .Add(fToolRectBtn)
        .AddStrut(12.0f)
        .Add(pageLabel)
        .Add(fRotateLeftBtn)
        .Add(fRotateRightBtn)
        .Add(fDeletePageBtn)
        .AddGlue()
        .Add(fSaveButton)
        .End();

    // 4. Central PDF view in BScrollView
    fPDFView = new PDFView("pdfView");
    fScrollView = new BScrollView("pdfScrollView", fPDFView, 0, true, true, B_PLAIN_BORDER);

    // 5. Build Sidebar TabView
    _BuildSidebar();

    // 6. Assemble everything using SplitView
    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
        .Add(fMenuBar)
        .Add(fNavToolBar)
        .Add(fEditToolBar)
        .AddSplit(B_HORIZONTAL, B_USE_SMALL_SPACING)
            .Add(fSidebarTabView, 0.28f)
            .Add(fScrollView, 0.72f)
        .End()
        .End();
}

void
MainWindow::MessageReceived(BMessage* message)
{
    switch (message->what) {
        case MSG_FILE_OPEN:
            if (fOpenPanel != nullptr)
                fOpenPanel->Show();
            break;

        case B_REFS_RECEIVED:
        {
            entry_ref ref;
            if (message->FindRef("refs", &ref) == B_OK)
                OpenFile(ref);
            break;
        }

        case MSG_FILE_SAVE:
            if (fPDFView->Document() != nullptr && strlen(fPDFView->Document()->FilePath()) > 0) {
                SaveFile(fPDFView->Document()->FilePath());
            } else if (fSavePanel != nullptr) {
                fSavePanel->Show();
            }
            break;

        case MSG_FILE_SAVE_AS:
            if (fSavePanel != nullptr)
                fSavePanel->Show();
            break;

        case B_SAVE_REQUESTED:
        {
            entry_ref dirRef;
            const char* name = nullptr;
            if (message->FindRef("directory", &dirRef) == B_OK &&
                message->FindString("name", &name) == B_OK) {
                BPath path(&dirRef);
                path.Append(name);
                SaveFile(path.Path());
            }
            break;
        }

        case MSG_FILE_CLOSE:
            fPDFView->SetDocument(nullptr);
            fObjectEditor->SetDocument(nullptr);
            fFormManager->SetDocument(nullptr);
            fSecurity->SetDocument(nullptr);
            SetTitle("Docrobat - PDF Editor Professionale");
            _UpdateControls();
            _UpdateSidebarData();
            break;

        case MSG_PAGE_PREV:
            fPDFView->PreviousPage();
            _UpdateControls();
            _UpdateSidebarData();
            break;

        case MSG_PAGE_NEXT:
            fPDFView->NextPage();
            _UpdateControls();
            _UpdateSidebarData();
            break;

        case MSG_ZOOM_IN:
            fPDFView->SetZoom(fPDFView->Zoom() * 1.25f);
            _UpdateControls();
            break;

        case MSG_ZOOM_OUT:
            fPDFView->SetZoom(fPDFView->Zoom() / 1.25f);
            _UpdateControls();
            break;

        case MSG_ZOOM_100:
            fPDFView->SetZoom(1.0f);
            _UpdateControls();
            break;

        case MSG_ZOOM_FIT:
            if (fPDFView->Bitmap() != nullptr) {
                float viewHeight = fPDFView->Bounds().Height() - 40.0f;
                float origHeight = fPDFView->Bitmap()->Bounds().Height() / fPDFView->Zoom();
                if (origHeight > 0.0f) {
                    fPDFView->SetZoom(viewHeight / origHeight);
                    _UpdateControls();
                }
            }
            break;

        case MSG_ROTATE_LEFT:
            fPDFView->RotateCurrentPage(270);
            _UpdateControls();
            _UpdateSidebarData();
            break;

        case MSG_ROTATE_RIGHT:
            fPDFView->RotateCurrentPage(90);
            _UpdateControls();
            _UpdateSidebarData();
            break;

        case MSG_DELETE_PAGE:
            if (fPDFView->PageCount() <= 1) {
                BAlert* alert = new BAlert("Docrobat",
                    "Impossibile eliminare l'unica pagina rimasta nel documento.", "OK");
                alert->Go();
            } else {
                BString prompt;
                prompt.SetToFormat("Sei sicuro di voler eliminare la pagina %d?",
                    static_cast<int>(fPDFView->CurrentPage() + 1));
                BAlert* alert = new BAlert("Docrobat", prompt.String(), "Annulla", "Elimina", nullptr,
                    B_WIDTH_AS_USUAL, B_WARNING_ALERT);
                alert->SetShortcut(0, B_ESCAPE);
                if (alert->Go() == 1) {
                    fPDFView->DeleteCurrentPage();
                    _UpdateControls();
                    _UpdateSidebarData();
                }
            }
            break;

        case MSG_TOOL_VIEW:
            fPDFView->SetToolMode(MODE_VIEW);
            _UpdateToolButtons();
            break;

        case MSG_TOOL_HIGHLIGHT:
            fPDFView->SetToolMode(MODE_HIGHLIGHT);
            _UpdateToolButtons();
            break;

        case MSG_TOOL_TEXT:
            fPDFView->SetToolMode(MODE_ANNOTATE_TEXT);
            _UpdateToolButtons();
            break;

        case MSG_TOOL_RECT:
            fPDFView->SetToolMode(MODE_DRAW_RECT);
            _UpdateToolButtons();
            break;

        case MSG_TOGGLE_SIDEBAR:
            _ToggleSidebar();
            break;

        case MSG_SELECT_THUMBNAIL:
        {
            int32 selected = fThumbnailList->CurrentSelection();
            if (selected >= 0 && selected < fPDFView->PageCount()) {
                fPDFView->SetCurrentPage(selected);
                _UpdateControls();
            }
            break;
        }

        case MSG_SELECT_BOOKMARK:
        {
            int32 selected = fBookmarksTree->CurrentSelection();
            if (selected >= 0 && selected < fPDFView->PageCount()) {
                fPDFView->SetCurrentPage(selected);
                _UpdateControls();
            }
            break;
        }

        case MSG_APPLY_FORM:
            if (fPDFView->Document() != nullptr) {
                fFormManager->AttachControlsToView(fPDFView, fPDFView->CurrentPage(), fPDFView->Bounds());
                BAlert* alert = new BAlert("Docrobat",
                    "Campi modulo interattivi attivati sulla vista corrente!", "OK");
                alert->Go();
            }
            break;

        case MSG_SECURITY_UNLOCK:
            if (fSecurity->Unlock("secret")) {
                BAlert* alert = new BAlert("Docrobat", "Documento sbloccato con successo!", "OK");
                alert->Go();
                _UpdateSidebarData();
            } else {
                BAlert* alert = new BAlert("Docrobat",
                    "Password errata o documento non protetto.", "OK");
                alert->Go();
            }
            break;

        case MSG_SECURITY_PROTECT:
        {
            DocumentPermissions perms = fSecurity->GetPermissions();
            perms.canModify = false;
            fSecurity->ProtectDocument("user", "admin", perms);
            BAlert* alert = new BAlert("Docrobat",
                "Restrizioni di sicurezza applicate con successo.", "OK");
            alert->Go();
            _UpdateSidebarData();
            break;
        }

        case B_ABOUT_REQUESTED:
            be_app->PostMessage(B_ABOUT_REQUESTED);
            break;

        default:
            BWindow::MessageReceived(message);
            break;
    }
}

bool
MainWindow::QuitRequested()
{
    be_app->PostMessage(B_QUIT_REQUESTED);
    return true;
}

void
MainWindow::OpenFile(const entry_ref& ref)
{
    BPath path(&ref);
    if (path.InitCheck() == B_OK)
        OpenFile(path.Path());
}

void
MainWindow::OpenFile(const char* path)
{
    if (path == nullptr)
        return;

    if (fPDFView->LoadDocument(path)) {
        fObjectEditor->SetDocument(fPDFView->Document());
        fFormManager->SetDocument(fPDFView->Document());
        fSecurity->SetDocument(fPDFView->Document());

        BPath p(path);
        BString title;
        title.SetToFormat("Docrobat - %s", p.Leaf());
        SetTitle(title.String());

        _UpdateControls();
        _UpdateSidebarData();
    } else {
        BAlert* alert = new BAlert("Docrobat",
            "Impossibile aprire il file PDF selezionato o file protetto da password.",
            "OK", nullptr, nullptr, B_WIDTH_AS_USUAL, B_STOP_ALERT);
        alert->Go();
    }
}

void
MainWindow::SaveFile(const char* path)
{
    if (path == nullptr || fPDFView->Document() == nullptr)
        return;

    if (fPDFView->Document()->SaveAs(path)) {
        BPath p(path);
        BString title;
        title.SetToFormat("Docrobat - %s", p.Leaf());
        SetTitle(title.String());

        BAlert* alert = new BAlert("Docrobat", "Documento salvato con successo!", "OK");
        alert->Go();
    } else {
        BAlert* alert = new BAlert("Docrobat",
            "Errore durante il salvataggio del documento PDF.",
            "OK", nullptr, nullptr, B_WIDTH_AS_USUAL, B_STOP_ALERT);
        alert->Go();
    }
}

void
MainWindow::_ToggleSidebar()
{
    fSidebarVisible = !fSidebarVisible;
    fSidebarTabView->SetExplicitHidden(!fSidebarVisible);
    fSidebarToggleBtn->SetLabel(fSidebarVisible ? "📑 Pannello" : "📑 Mostra");
}

void
MainWindow::_UpdateControls()
{
    int32 current = fPDFView->CurrentPage();
    int32 total = fPDFView->PageCount();

    if (total > 0) {
        BString info;
        info.SetToFormat("Pagina %d di %d", static_cast<int>(current + 1), static_cast<int>(total));
        fPageInfoView->SetText(info.String());

        BString zoomStr;
        zoomStr.SetToFormat("%d%%", static_cast<int>(std::round(fPDFView->Zoom() * 100.0f)));
        fZoomInfoView->SetText(zoomStr.String());

        fPrevButton->SetEnabled(current > 0);
        fNextButton->SetEnabled(current + 1 < total);
        fZoomInButton->SetEnabled(true);
        fZoomOutButton->SetEnabled(true);
        fZoomFitButton->SetEnabled(true);
        fZoomResetButton->SetEnabled(true);

        fRotateLeftBtn->SetEnabled(true);
        fRotateRightBtn->SetEnabled(true);
        fDeletePageBtn->SetEnabled(total > 1);
        fSaveButton->SetEnabled(true);
        fApplyFormBtn->SetEnabled(true);
    } else {
        fPageInfoView->SetText("Nessun documento");
        fZoomInfoView->SetText("100%");
        fPrevButton->SetEnabled(false);
        fNextButton->SetEnabled(false);
        fZoomInButton->SetEnabled(false);
        fZoomOutButton->SetEnabled(false);
        fZoomFitButton->SetEnabled(false);
        fZoomResetButton->SetEnabled(false);

        fRotateLeftBtn->SetEnabled(false);
        fRotateRightBtn->SetEnabled(false);
        fDeletePageBtn->SetEnabled(false);
        fSaveButton->SetEnabled(false);
        fApplyFormBtn->SetEnabled(false);
    }
}

void
MainWindow::_UpdateToolButtons()
{
    ToolMode mode = fPDFView->GetToolMode();
    fToolViewBtn->SetLabel(mode == MODE_VIEW ? "[ ✋ Vista ]" : "✋ Vista");
    fToolHighlightBtn->SetLabel(mode == MODE_HIGHLIGHT ? "[ 🖊 Evidenzia ]" : "🖊 Evidenzia");
    fToolTextBtn->SetLabel(mode == MODE_ANNOTATE_TEXT ? "[ 📝 Nota ]" : "📝 Nota");
    fToolRectBtn->SetLabel(mode == MODE_DRAW_RECT ? "[ ▭ Rettangolo ]" : "▭ Rettangolo");
}

void
MainWindow::_UpdateSidebarData()
{
    int32 count = fPDFView->PageCount();
    int32 cur = fPDFView->CurrentPage();

    // 1. Aggiorna Tab Miniature
    fThumbnailList->MakeEmpty();
    for (int32 i = 0; i < count; ++i) {
        BSize sz = fPDFView->Document() ? fPDFView->Document()->PageSize(i) : BSize(0, 0);
        BString itemStr;
        itemStr.SetToFormat("Pagina %d  (%d x %d pt)",
            static_cast<int>(i + 1), static_cast<int>(sz.width), static_cast<int>(sz.height));
        fThumbnailList->AddItem(new BStringItem(itemStr.String()));
    }
    if (cur >= 0 && cur < count)
        fThumbnailList->Select(cur);

    // 2. Aggiorna Tab Segnalibri
    fBookmarksTree->MakeEmpty();
    if (count > 0) {
        BStringItem* root = new BStringItem("Indice Documento");
        fBookmarksTree->AddItem(root);
        fBookmarksTree->AddUnder(new BStringItem("Copertina e Frontespizio"), root);
        fBookmarksTree->AddUnder(new BStringItem("Contenuto Principale"), root);
        if (count > 2)
            fBookmarksTree->AddUnder(new BStringItem("Appendice e Riferimenti"), root);
        fBookmarksTree->Expand(root);
    }

    // 3. Aggiorna Tab Moduli
    fFormFieldsList->MakeEmpty();
    if (count > 0 && fFormManager != nullptr) {
        auto fields = fFormManager->GetFields(cur);
        for (const auto& f : fields) {
            BString fieldStr;
            const char* typeName = "Testo";
            if (f.type == FIELD_CHECKBOX) typeName = "Spunta";
            else if (f.type == FIELD_CHOICE) typeName = "Scelta";
            fieldStr.SetToFormat("[%s] %s = '%s'", typeName, f.name.String(), f.value.String());
            fFormFieldsList->AddItem(new BStringItem(fieldStr.String()));
        }
        if (fields.empty()) {
            fFormFieldsList->AddItem(new BStringItem("Nessun campo su questa pagina"));
        }
    }

    // 4. Aggiorna Tab Sicurezza
    if (count > 0 && fPDFView->Document() != nullptr) {
        BPath p(fPDFView->Document()->FilePath());
        BString titleStr;
        titleStr.SetToFormat("File: %s", p.Leaf());
        fDocTitleView->SetText(titleStr.String());

        BString pagesStr;
        pagesStr.SetToFormat("Totale pagine: %d", static_cast<int>(count));
        fDocPagesView->SetText(pagesStr.String());

        DocumentPermissions perms = fSecurity->GetPermissions();
        BString secStr;
        secStr.SetToFormat("Sicurezza: %s", perms.encryptionAlgorithm.String());
        fSecurityStatusView->SetText(secStr.String());

        BString permStr = "Permessi: ";
        if (perms.canPrint) permStr << "Stampa ";
        if (perms.canModify) permStr << "Modifica ";
        if (perms.canCopy) permStr << "Copia ";
        if (perms.canAnnotate) permStr << "Note ";
        fPermissionsView->SetText(permStr.String());
    } else {
        fDocTitleView->SetText("Documento: Nessuno");
        fDocPagesView->SetText("Totale pagine: 0");
        fSecurityStatusView->SetText("Sicurezza: Nessuna restrizione");
        fPermissionsView->SetText("Permessi: Tutti abilitati");
    }
}
