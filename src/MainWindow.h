#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <Window.h>
#include <FilePanel.h>
#include <Entry.h>

class BMenuBar;
class BGroupView;
class BButton;
class BStringView;
class BScrollView;
class PDFView;

// Haiku 4-character message constants
enum {
    MSG_FILE_OPEN       = 'DOCO',
    MSG_FILE_CLOSE      = 'DOCC',
    MSG_FILE_SAVE       = 'DOCS',
    MSG_FILE_SAVE_AS    = 'DOCA',
    MSG_PAGE_PREV       = 'PGUP',
    MSG_PAGE_NEXT       = 'PGDN',
    MSG_ZOOM            = 'ZOOM',
    MSG_ZOOM_IN         = 'ZMIN',
    MSG_ZOOM_OUT        = 'ZOUT',
    MSG_ZOOM_100        = 'Z100',
    MSG_ZOOM_FIT        = 'ZFPG'
};

class MainWindow : public BWindow {
public:
                            MainWindow();
    virtual                 ~MainWindow();

    virtual void            MessageReceived(BMessage* message) override;
    virtual bool            QuitRequested() override;

    void                    OpenFile(const entry_ref& ref);
    void                    OpenFile(const char* path);

private:
    void                    _BuildLayout();
    void                    _UpdateControls();

    BMenuBar*               fMenuBar;
    BGroupView*             fToolBar;
    PDFView*                fPDFView;
    BScrollView*            fScrollView;
    BFilePanel*             fOpenPanel;

    BButton*                fOpenButton;
    BButton*                fPrevButton;
    BButton*                fNextButton;
    BStringView*            fPageInfoView;
    BButton*                fZoomOutButton;
    BStringView*            fZoomInfoView;
    BButton*                fZoomInButton;
    BButton*                fZoomFitButton;
    BButton*                fZoomResetButton;
};

#endif // MAIN_WINDOW_H
