#include "MainWindow.h"
#include "PDFView.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <GroupView.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Path.h>
#include <ScrollView.h>
#include <String.h>
#include <StringView.h>

#include <cmath>

MainWindow::MainWindow()
    : BWindow(BRect(100, 100, 950, 750), "Docrobat", B_TITLED_WINDOW,
        B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS),
      fMenuBar(nullptr),
      fToolBar(nullptr),
      fPDFView(nullptr),
      fScrollView(nullptr),
      fOpenPanel(nullptr),
      fOpenButton(nullptr),
      fPrevButton(nullptr),
      fNextButton(nullptr),
      fPageInfoView(nullptr),
      fZoomOutButton(nullptr),
      fZoomInfoView(nullptr),
      fZoomInButton(nullptr),
      fZoomFitButton(nullptr),
      fZoomResetButton(nullptr)
{
    _BuildLayout();

    fOpenPanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), nullptr, B_FILE_NODE, false);

    _UpdateControls();
}

MainWindow::~MainWindow()
{
    delete fOpenPanel;
}

void
MainWindow::_BuildLayout()
{
    // 1. Menu Bar
    fMenuBar = new BMenuBar("mainMenuBar");

    BMenu* fileMenu = new BMenu("File");
    fileMenu->AddItem(new BMenuItem("Apri...", new BMessage(MSG_FILE_OPEN), 'O'));
    fileMenu->AddItem(new BMenuItem("Chiudi", new BMessage(MSG_FILE_CLOSE), 'W'));
    fileMenu->AddSeparatorItem();
    fileMenu->AddItem(new BMenuItem("Salva", new BMessage(MSG_FILE_SAVE), 'S'));
    fileMenu->AddItem(new BMenuItem("Salva come...", new BMessage(MSG_FILE_SAVE_AS)));
    fileMenu->AddSeparatorItem();
    fileMenu->AddItem(new BMenuItem("Informazioni su Docrobat", new BMessage(B_ABOUT_REQUESTED)));
    fileMenu->AddSeparatorItem();
    fileMenu->AddItem(new BMenuItem("Esci", new BMessage(B_QUIT_REQUESTED), 'Q'));
    fMenuBar->AddItem(fileMenu);

    BMenu* viewMenu = new BMenu("Visualizza");
    viewMenu->AddItem(new BMenuItem("Ingrandisci", new BMessage(MSG_ZOOM_IN), '+'));
    viewMenu->AddItem(new BMenuItem("Rimpicciolisci", new BMessage(MSG_ZOOM_OUT), '-'));
    viewMenu->AddItem(new BMenuItem("Dimensione reale (100%)", new BMessage(MSG_ZOOM_100), '0'));
    viewMenu->AddItem(new BMenuItem("Adatta a pagina", new BMessage(MSG_ZOOM_FIT)));
    fMenuBar->AddItem(viewMenu);

    BMenu* goMenu = new BMenu("Vai");
    goMenu->AddItem(new BMenuItem("Pagina precedente", new BMessage(MSG_PAGE_PREV), B_PAGE_UP));
    goMenu->AddItem(new BMenuItem("Pagina successiva", new BMessage(MSG_PAGE_NEXT), B_PAGE_DOWN));
    fMenuBar->AddItem(goMenu);

    // 2. Navigation & Zoom Toolbar
    fToolBar = new BGroupView(B_HORIZONTAL, B_USE_SMALL_SPACING);
    fToolBar->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

    fOpenButton = new BButton("openButton", "Apri...", new BMessage(MSG_FILE_OPEN));
    fPrevButton = new BButton("prevButton", "◀ Prec.", new BMessage(MSG_PAGE_PREV));
    fNextButton = new BButton("nextButton", "Succ. ▶", new BMessage(MSG_PAGE_NEXT));
    fPageInfoView = new BStringView("pageInfoView", "Nessun documento");

    fZoomOutButton = new BButton("zoomOutButton", "−", new BMessage(MSG_ZOOM_OUT));
    fZoomInfoView = new BStringView("zoomInfoView", "100%");
    fZoomInButton = new BButton("zoomInButton", "+", new BMessage(MSG_ZOOM_IN));
    fZoomResetButton = new BButton("zoomResetButton", "100%", new BMessage(MSG_ZOOM_100));
    fZoomFitButton = new BButton("zoomFitButton", "Adatta", new BMessage(MSG_ZOOM_FIT));

    BLayoutBuilder::Group<>(fToolBar, B_HORIZONTAL, B_USE_SMALL_SPACING)
        .SetInsets(B_USE_SMALL_INSETS)
        .Add(fOpenButton)
        .AddStrut(12.0f)
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

    // 3. Central PDF view in BScrollView
    fPDFView = new PDFView("pdfView");
    fScrollView = new BScrollView("pdfScrollView", fPDFView, 0, true, true, B_PLAIN_BORDER);

    // 4. Assemble layout
    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
        .Add(fMenuBar)
        .Add(fToolBar)
        .Add(fScrollView)
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

        case MSG_FILE_CLOSE:
            fPDFView->SetDocument(nullptr);
            SetTitle("Docrobat");
            _UpdateControls();
            break;

        case MSG_PAGE_PREV:
            fPDFView->PreviousPage();
            _UpdateControls();
            break;

        case MSG_PAGE_NEXT:
            fPDFView->NextPage();
            _UpdateControls();
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
        BPath p(path);
        BString title;
        title.SetToFormat("Docrobat - %s", p.Leaf());
        SetTitle(title.String());
        _UpdateControls();
    } else {
        BAlert* alert = new BAlert("Docrobat",
            "Impossibile aprire il file PDF selezionato o file non valido/protetto da password.",
            "OK", nullptr, nullptr, B_WIDTH_AS_USUAL, B_STOP_ALERT);
        alert->Go();
    }
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
    } else {
        fPageInfoView->SetText("Nessun documento");
        fZoomInfoView->SetText("100%");
        fPrevButton->SetEnabled(false);
        fNextButton->SetEnabled(false);
        fZoomInButton->SetEnabled(false);
        fZoomOutButton->SetEnabled(false);
        fZoomFitButton->SetEnabled(false);
        fZoomResetButton->SetEnabled(false);
    }
}
