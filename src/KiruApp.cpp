#include "KiruWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Entry.h>
#include <Message.h>
#include <Roster.h>

#include <cstdlib>
#include <string>
#include <unistd.h>

static bool FFmpegIsInstalled() {
	const char* path = std::getenv("PATH");
	if (path == nullptr)
		return false;

	std::string searchPath(path);
	size_t start = 0;
	while (start <= searchPath.size()) {
		size_t end = searchPath.find(':', start);
		std::string directory = searchPath.substr(start, end - start);
		std::string candidate = directory.empty()
			? "ffmpeg" : directory + "/ffmpeg";
		if (access(candidate.c_str(), X_OK) == 0)
			return true;
		if (end == std::string::npos)
			break;
		start = end + 1;
	}
	return false;
}
//---------------------------------------------------------------------------------------------------------------------------------//


static status_t LaunchFFmpegInstaller() {
	const char* arguments[] = {
		"-t",
		"Install FFmpeg for Kiru",
		"/bin/sh",
		"-c",
		"pkgman install ffmpeg; result=$?; printf '\\n'; "
		"if [ $result -eq 0 ]; then "
		"echo 'FFmpeg was installed. You can return to Kiru.'; "
		"else echo 'FFmpeg could not be installed.'; fi; "
		"printf '\\nPress Enter to close Terminal...'; read line; exit $result"
	};
	return be_roster->Launch("application/x-vnd.Haiku-Terminal", 5, arguments);
}
//---------------------------------------------------------------------------------------------------------------------------------//

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


void KiruApp::ArgvReceived(int32 argc, char** argv) {
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


void KiruApp::RefsReceived(BMessage* message) {
	if (fWindow != nullptr)
		fWindow->PostMessage(message);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void KiruApp::ReadyToRun() {
	if (!FFmpegIsInstalled()) {
		BAlert* alert = new BAlert("FFmpeg required",
			"Kiru needs FFmpeg to create chopped video files, but it was not "
			"found in PATH.\n\nInstall it now by opening Terminal and running:\n\n"
			"pkgman install ffmpeg",
			"Cancel", "Install FFmpeg", nullptr, B_WIDTH_AS_USUAL,
			B_WARNING_ALERT);
		alert->SetShortcut(0, B_ESCAPE);
		alert->SetShortcut(1, B_ENTER);
		if (alert->Go() == 1 && LaunchFFmpegInstaller() != B_OK) {
			(new BAlert("Could not open Terminal",
				"Kiru could not open Terminal. Run 'pkgman install ffmpeg' "
				"manually, then try again.", "OK"))->Go();
		}
	}
	fWindow = new KiruWindow();
	fWindow->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


int main() {
	KiruApp application;
	application.Run();
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
