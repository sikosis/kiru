#include "KiruWindow.h"

#include "AboutWindow.h"
#include "Messages.h"
#include "VideoCutter.h"
#include "VideoView.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <Entry.h>
#include <FilePanel.h>
#include <GroupLayoutBuilder.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <MessageRunner.h>
#include <Path.h>
#include <Roster.h>
#include <Slider.h>
#include <StringView.h>

#include <algorithm>
#include <cstdio>

static const int32 kTimelineResolution = 1000000;
static const char* kMediaPlayerSignature = "application/x-vnd.Haiku-MediaPlayer";
static const char* kVLCSignature = "application/x-vnd.videolan-vlc";

KiruWindow::KiruWindow()
	:
	BWindow(BRect(100, 100, 980, 680), "Kiru", B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS | B_ASYNCHRONOUS_CONTROLS),
	fInPoint(-1),
	fOutPoint(-1),
	fScrubPosition(0),
	fPlaying(false),
	fCutting(false),
	fScrubbing(false)
{
	fVideoView = new VideoView();
	fTimeline = new BSlider("timeline", nullptr, new BMessage(MSG_SEEK), 0,
		kTimelineResolution, B_HORIZONTAL);
	fTimeline->SetModificationMessage(new BMessage(MSG_SCRUB));
	fTimeline->SetHashMarks(B_HASH_MARKS_NONE);
	fTimeLabel = new BStringView("time", "00:00.000 / 00:00.000");
	fMarksLabel = new BStringView("marks", "IN  --:--.---     OUT  --:--.---");
	fStatusLabel = new BStringView("status", "Drop a video here, or press L to load one.");
	BStringView* shortcuts = new BStringView("shortcuts",
		"Shortcuts: L Load · I/O Mark · Space/P Preview · K Chop · ←/→ 1s · "
		"Shift+←/→ 5s · ,/. Frame · Home/End");
	fPlayButton = new BButton("play", "Play / Preview", new BMessage(MSG_TOGGLE_PLAY));
	fCutButton = new BButton("cut", "Chop  K", new BMessage(MSG_CUT));
	fCutButton->SetEnabled(false);

	BButton* openButton = new BButton("open", "Load  L", new BMessage(MSG_OPEN));
	BButton* inButton = new BButton("in", "Mark In  I", new BMessage(MSG_MARK_IN));
	BButton* outButton = new BButton("out", "Mark Out  O", new BMessage(MSG_MARK_OUT));
	BMenuBar* menuBar = new BMenuBar("menu bar");
	BMenu* applicationMenu = new BMenu("Kiru");
	applicationMenu->AddItem(new BMenuItem("About Kiru" B_UTF8_ELLIPSIS,
		new BMessage(B_ABOUT_REQUESTED)));
	applicationMenu->AddSeparatorItem();
	applicationMenu->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED),
		'Q', B_COMMAND_KEY));
	menuBar->AddItem(applicationMenu);

	SetLayout(new BGroupLayout(B_VERTICAL, 0));
	AddChild(menuBar);
	AddChild(BGroupLayoutBuilder(B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(12.0f, 12.0f, 12.0f, 12.0f)
		.Add(fVideoView, 1.0f)
		.Add(fTimeline)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fTimeLabel)
			.AddGlue()
			.Add(fMarksLabel)
		.End()
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(openButton)
			.Add(fPlayButton)
			.Add(inButton)
			.Add(outButton)
			.Add(fCutButton)
		.End()
		.Add(fStatusLabel)
		.Add(shortcuts));

	fOpenPanel = std::make_unique<BFilePanel>(B_OPEN_PANEL, new BMessenger(this),
		nullptr, B_FILE_NODE, false);
	fTicker = std::make_unique<BMessageRunner>(BMessenger(this), new BMessage(MSG_TICK),
		40000);
	fTimeline->SetEnabled(false);
	SetSizeLimits(560, 4000, 380, 4000);
}
//---------------------------------------------------------------------------------------------------------------------------------//


KiruWindow::~KiruWindow()
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KiruWindow::DispatchMessage(BMessage* message, BHandler* handler)
{
	if (message->what == B_KEY_DOWN) {
		int32 keyModifiers = 0;
		message->FindInt32("modifiers", &keyModifiers);
		if ((keyModifiers & (B_COMMAND_KEY | B_CONTROL_KEY | B_OPTION_KEY)) == 0) {
			const char* bytes = nullptr;
			if (message->FindString("bytes", &bytes) == B_OK && bytes[0] != '\0') {
				uint32 action = 0;
				switch (bytes[0]) {
					case 'i':
					case 'I':
						action = MSG_MARK_IN;
						break;
					case 'o':
					case 'O':
						action = MSG_MARK_OUT;
						break;
					case 'k':
					case 'K':
						action = MSG_CUT;
						break;
					case 'p':
					case 'P':
					case ' ':
						action = MSG_TOGGLE_PLAY;
						break;
					case 'j':
					case 'J':
						action = MSG_BACK_FIVE;
						break;
					case 'l':
					case 'L':
						action = MSG_OPEN;
						break;
					case ',':
						action = MSG_BACK_FRAME;
						break;
					case '.':
						action = MSG_FORWARD_FRAME;
						break;
					case B_LEFT_ARROW:
						action = (keyModifiers & B_SHIFT_KEY) != 0
							? MSG_BACK_FIVE : MSG_BACK_ONE;
						break;
					case B_RIGHT_ARROW:
						action = (keyModifiers & B_SHIFT_KEY) != 0
							? MSG_FORWARD_FIVE : MSG_FORWARD_ONE;
						break;
					case B_HOME:
						action = MSG_TO_START;
						break;
					case B_END:
						action = MSG_TO_END;
						break;
				}
				if (action != 0) {
					PostMessage(action);
					return;
				}
			}
		}
	}
	BWindow::DispatchMessage(message, handler);
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
KiruWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KiruWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_OPEN:
			fOpenPanel->Show();
			break;
		case B_REFS_RECEIVED:
		case B_SIMPLE_DATA: {
			entry_ref reference;
			if (message->FindRef("refs", &reference) == B_OK) {
				BPath path(&reference);
				OpenFile(path.Path());
			}
			break;
		}
		case MSG_SCRUB:
			if (fPlayer.IsOpen()) {
				fPlaying = false;
				fScrubbing = true;
				fScrubPosition = static_cast<bigtime_t>(fTimeline->Value())
					* fPlayer.Duration() / kTimelineResolution;
				UpdateInterface();
			}
			break;
		case MSG_SEEK:
			if (fPlayer.IsOpen()) {
				bigtime_t position = static_cast<bigtime_t>(fTimeline->Value())
					* fPlayer.Duration() / kTimelineResolution;
				fScrubbing = false;
				Seek(position);
			}
			break;
		case MSG_TOGGLE_PLAY:
			TogglePlayback();
			break;
		case MSG_BACK_ONE:
			Seek(CurrentPosition() - 1000000);
			break;
		case MSG_FORWARD_ONE:
			Seek(CurrentPosition() + 1000000);
			break;
		case MSG_BACK_FIVE:
			Seek(CurrentPosition() - 5000000);
			break;
		case MSG_FORWARD_FIVE:
			Seek(CurrentPosition() + 5000000);
			break;
		case MSG_BACK_FRAME:
			Seek(CurrentPosition()
				- static_cast<bigtime_t>(1000000.0f / fPlayer.FrameRate()));
			break;
		case MSG_FORWARD_FRAME:
			Seek(CurrentPosition()
				+ static_cast<bigtime_t>(1000000.0f / fPlayer.FrameRate()));
			break;
		case MSG_TO_START:
			Seek(0);
			break;
		case MSG_TO_END:
			Seek(fPlayer.Duration());
			break;
		case MSG_MARK_IN:
			SetMark(true);
			break;
		case MSG_MARK_OUT:
			SetMark(false);
			break;
		case MSG_CUT:
			Chop();
			break;
		case B_ABOUT_REQUESTED:
			ShowAbout();
			break;
		case MSG_TICK:
			Tick();
			break;
		case MSG_CUT_FINISHED: {
			fCutting = false;
			int32 status = message->GetInt32("status", -1);
			const char* output = message->GetString("output", "");
			if (status == 0) {
				BString text("Saved ");
				text << output;
				SetStatus(text.String());
				UpdateInterface();
				ShowCutFinished(output);
			} else if (status == 127) {
				SetStatus("FFmpeg was not found. Install it from HaikuDepot.");
			} else {
				SetStatus("FFmpeg could not chop this video.");
			}
			if (status != 0)
				UpdateInterface();
			break;
		}
		default:
			BWindow::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KiruWindow::OpenFile(const char* path)
{
	fPlaying = false;
	fScrubbing = false;
	fScrubPosition = 0;
	fInPoint = -1;
	fOutPoint = -1;
	BString error;
	status_t status = fPlayer.Open(path, error);
	if (status != B_OK) {
		(new BAlert("Open video", error.String(), "OK"))->Go();
		return;
	}
	fVideoView->SetBitmap(fPlayer.Bitmap());
	bigtime_t tickInterval = static_cast<bigtime_t>(1000000.0f / fPlayer.FrameRate());
	tickInterval = std::clamp(tickInterval, static_cast<bigtime_t>(10000),
		static_cast<bigtime_t>(100000));
	fTicker = std::make_unique<BMessageRunner>(BMessenger(this), new BMessage(MSG_TICK),
		tickInterval);
	fTimeline->SetEnabled(true);
	BPath videoPath(path);
	BString title("Kiru — ");
	title << videoPath.Leaf();
	SetTitle(title.String());
	SetStatus("Video loaded. Set in and out points, then preview or chop.");
	UpdateInterface();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KiruWindow::Seek(bigtime_t position, bool precise)
{
	if (!fPlayer.IsOpen())
		return;
	fScrubbing = false;
	if (!precise)
		fPlaying = false;
	status_t status = fPlayer.Seek(position, precise);
	if (status != B_OK) {
		SetStatus("Could not seek to that position.");
		return;
	}
	fVideoView->Invalidate();
	UpdateInterface();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KiruWindow::TogglePlayback()
{
	if (!fPlayer.IsOpen())
		return;
	if (!fPlaying) {
		bigtime_t start = CurrentPosition();
		if (fInPoint >= 0 && fOutPoint > fInPoint
			&& (start < fInPoint || start >= fOutPoint)) {
			start = fInPoint;
		}
		Seek(start, true);
	}
	fPlaying = !fPlaying;
	SetStatus(fPlaying ? "Previewing…" : "Paused.");
	UpdateInterface();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KiruWindow::SetMark(bool isInMark)
{
	if (!fPlayer.IsOpen())
		return;
	if (isInMark)
		fInPoint = CurrentPosition();
	else
		fOutPoint = CurrentPosition();
	if (fInPoint >= 0 && fOutPoint >= 0 && fInPoint >= fOutPoint)
		SetStatus("The out point must be after the in point.");
	else
		SetStatus(isInMark ? "In point set." : "Out point set.");
	UpdateInterface();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KiruWindow::Chop()
{
	if (!fPlayer.IsOpen() || fInPoint < 0 || fOutPoint <= fInPoint || fCutting)
		return;
	fPlaying = false;
	BString output;
	BString error;
	status_t status = VideoCutter::Start(fPlayer.Path(), fInPoint, fOutPoint,
		BMessenger(this), output, error);
	if (status != B_OK) {
		SetStatus(error.IsEmpty() ? "Could not start the cut." : error.String());
		return;
	}
	fCutting = true;
	BString text("Chopping to ");
	text << output;
	SetStatus(text.String());
	UpdateInterface();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KiruWindow::Tick()
{
	if (!fPlaying || !fPlayer.IsOpen())
		return;
	bool hasSelection = fOutPoint > fInPoint && fInPoint >= 0;
	bigtime_t end = hasSelection ? fOutPoint : fPlayer.Duration();
	bigtime_t frameDuration = std::max<bigtime_t>(1,
		static_cast<bigtime_t>(1000000.0f / fPlayer.FrameRate()));
	bigtime_t endSafetyMargin = hasSelection
		? frameDuration : std::max<bigtime_t>(frameDuration * 3, 100000);
	bigtime_t finalFramePosition = std::max<bigtime_t>(0, end - endSafetyMargin);
	if (fPlayer.Position() >= finalFramePosition || fPlayer.ReadFrame() != B_OK) {
		fPlaying = false;
		Seek(fInPoint >= 0 && fOutPoint > fInPoint ? fInPoint : 0);
		SetStatus("Preview finished.");
	}
	fVideoView->Invalidate();
	UpdateInterface();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KiruWindow::ShowAbout()
{
	AboutWindow* window = new AboutWindow();
	window->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void KiruWindow::ShowCutFinished(const char* output) {
	BPath outputPath(output);
	BString text("Saved ");
	text << outputPath.Leaf() << ".\n\nWhat would you like to do?";
	const char* playerButton = VLCIsInstalled()
		? "Open in Player" B_UTF8_ELLIPSIS : "Open in MediaPlayer";
	BAlert* alert = new BAlert("Chop finished", text.String(), "Done",
		playerButton, "Show in Tracker", B_WIDTH_AS_USUAL, B_INFO_ALERT);
	alert->SetShortcut(0, B_ESCAPE);
	alert->SetShortcut(1, B_ENTER);
	int32 choice = alert->Go();
	status_t status = B_OK;
	if (choice == 1)
		status = OpenInPlayer(output);
	else if (choice == 2)
		status = ShowInTracker(output);
	if (status != B_OK)
		SetStatus("The chopped video was saved, but the selected app could not be opened.");
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool KiruWindow::VLCIsInstalled() const {
	entry_ref reference;
	return be_roster->FindApp(kVLCSignature, &reference) == B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t KiruWindow::OpenInPlayer(const char* output) {
	const char* signature = kMediaPlayerSignature;
	if (VLCIsInstalled()) {
		BAlert* alert = new BAlert("Open chopped video",
			"Which player would you like to use?", "Cancel", "MediaPlayer",
			"VLC", B_WIDTH_AS_USUAL, B_INFO_ALERT);
		alert->SetShortcut(0, B_ESCAPE);
		alert->SetShortcut(1, B_ENTER);
		int32 choice = alert->Go();
		if (choice == 0)
			return B_OK;
		if (choice == 2)
			signature = kVLCSignature;
	}
	return OpenWithApplication(output, signature);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t KiruWindow::OpenWithApplication(const char* output,
	const char* signature) {
	entry_ref reference;
	status_t status = get_ref_for_path(output, &reference);
	if (status != B_OK)
		return status;
	BMessage refs(B_REFS_RECEIVED);
	status = refs.AddRef("refs", &reference);
	if (status != B_OK)
		return status;
	return be_roster->Launch(signature, &refs);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
KiruWindow::ShowInTracker(const char* output)
{
	BEntry file(output, true);
	status_t status = file.InitCheck();
	if (status != B_OK)
		return status;
	BEntry parent;
	status = file.GetParent(&parent);
	if (status != B_OK)
		return status;
	entry_ref parentReference;
	status = parent.GetRef(&parentReference);
	if (status != B_OK)
		return status;
	BMessage refs(B_REFS_RECEIVED);
	status = refs.AddRef("refs", &parentReference);
	if (status != B_OK)
		return status;
	node_ref fileNode;
	if (file.GetNodeRef(&fileNode) == B_OK) {
		refs.AddData("nodeRefToSelect", B_RAW_TYPE, &fileNode,
			sizeof(fileNode));
	}
	BMessenger tracker("application/x-vnd.Be-TRAK");
	if (tracker.IsValid())
		return tracker.SendMessage(&refs);
	return be_roster->Launch("application/x-vnd.Be-TRAK", &refs);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KiruWindow::UpdateInterface()
{
	if (!fPlayer.IsOpen())
		return;
	bigtime_t position = CurrentPosition();
	int32 value = fPlayer.Duration() > 0
		? static_cast<int32>(position * kTimelineResolution
			/ fPlayer.Duration()) : 0;
	if (!fScrubbing)
		fTimeline->SetValue(std::clamp(value, 0, kTimelineResolution));
	BString time = TimeText(position);
	time << " / " << TimeText(fPlayer.Duration());
	fTimeLabel->SetText(time.String());
	BString marks("IN  ");
	marks << (fInPoint >= 0 ? TimeText(fInPoint) : "--:--.---") << "     OUT  "
		<< (fOutPoint >= 0 ? TimeText(fOutPoint) : "--:--.---");
	fMarksLabel->SetText(marks.String());
	fPlayButton->SetLabel(fPlaying ? "Pause" : "Play / Preview");
	fCutButton->SetEnabled(fInPoint >= 0 && fOutPoint > fInPoint && !fCutting);
}
//---------------------------------------------------------------------------------------------------------------------------------//


bigtime_t
KiruWindow::CurrentPosition() const
{
	return fScrubbing ? fScrubPosition : fPlayer.Position();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
KiruWindow::SetStatus(const char* text)
{
	fStatusLabel->SetText(text);
}
//---------------------------------------------------------------------------------------------------------------------------------//


BString
KiruWindow::TimeText(bigtime_t time) const
{
	int64 milliseconds = std::max<bigtime_t>(0, time) / 1000;
	int64 hours = milliseconds / 3600000;
	int64 minutes = milliseconds / 60000 % 60;
	int64 seconds = milliseconds / 1000 % 60;
	milliseconds %= 1000;
	BString text;
	if (hours > 0)
		text.SetToFormat("%02" B_PRId64 ":%02" B_PRId64 ":%02" B_PRId64 ".%03" B_PRId64,
			hours, minutes, seconds, milliseconds);
	else
		text.SetToFormat("%02" B_PRId64 ":%02" B_PRId64 ".%03" B_PRId64,
			minutes, seconds, milliseconds);
	return text;
}
//---------------------------------------------------------------------------------------------------------------------------------//
