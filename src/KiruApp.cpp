#include "KiruWindow.h"

#include <Application.h>
#include <Entry.h>
#include <Message.h>

class KiruApp : public BApplication {
public:
	KiruApp();

	void ArgvReceived(int32 argc, char** argv) override;
	void RefsReceived(BMessage* message) override;
	void ReadyToRun() override;

private:
	KiruWindow* fWindow;
};

KiruApp::KiruApp()
	:
	BApplication("application/x-vnd.sikosis-kiru"),
	fWindow(nullptr)
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KiruApp::ArgvReceived(int32 argc, char** argv)
{
	if (argc < 2 || fWindow == nullptr)
		return;
	BMessage message(B_REFS_RECEIVED);
	entry_ref reference;
	if (get_ref_for_path(argv[1], &reference) == B_OK) {
		message.AddRef("refs", &reference);
		fWindow->PostMessage(&message);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KiruApp::RefsReceived(BMessage* message)
{
	if (fWindow != nullptr)
		fWindow->PostMessage(message);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KiruApp::ReadyToRun()
{
	fWindow = new KiruWindow();
	fWindow->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


int
main()
{
	KiruApp application;
	application.Run();
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
