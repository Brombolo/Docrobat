#include "App.h"
#include "MainWindow.h"

#include <AboutWindow.h>
#include <Entry.h>
#include <Path.h>

const char* kAppSignature = "application/x-vnd.Docrobat";

DocrobatApp::DocrobatApp()
    : BApplication(kAppSignature),
      fMainWindow(nullptr)
{
    fMainWindow = new MainWindow();
}

DocrobatApp::~DocrobatApp()
{
}

void
DocrobatApp::ReadyToRun()
{
    if (fMainWindow != nullptr)
        fMainWindow->Show();
}

void
DocrobatApp::RefsReceived(BMessage* message)
{
    if (fMainWindow != nullptr)
        fMainWindow->PostMessage(message);
}

void
DocrobatApp::ArgvReceived(int32 argc, char** argv)
{
    if (argc > 1 && fMainWindow != nullptr) {
        BEntry entry(argv[1], true);
        entry_ref ref;
        if (entry.GetRef(&ref) == B_OK)
            fMainWindow->OpenFile(ref);
        else
            fMainWindow->OpenFile(argv[1]);
    }
}

void
DocrobatApp::AboutRequested()
{
    BAboutWindow* about = new BAboutWindow("Docrobat", kAppSignature);
    about->AddDescription("Visualizzatore ed editor PDF nativo per Haiku OS.");
    about->AddCopyright(2026, "Docrobat Contributors");
    about->Show();
}
