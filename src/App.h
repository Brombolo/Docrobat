#ifndef DOCROBAT_APP_H
#define DOCROBAT_APP_H

#include <Application.h>

class MainWindow;

extern const char* kAppSignature;

class DocrobatApp : public BApplication {
public:
                            DocrobatApp();
    virtual                 ~DocrobatApp();

    virtual void            ReadyToRun() override;
    virtual void            RefsReceived(BMessage* message) override;
    virtual void            ArgvReceived(int32 argc, char** argv) override;
    virtual void            AboutRequested() override;

private:
    MainWindow*             fMainWindow;
};

#endif // DOCROBAT_APP_H
