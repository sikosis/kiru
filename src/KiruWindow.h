#ifndef KIRU_WINDOW_H
#define KIRU_WINDOW_H

#include "VideoPlayer.h"

#include <Window.h>

#include <memory>

class BButton;
class BFilePanel;
class BMessageRunner;
class BSlider;
class BStringView;
class VideoView;

class KiruWindow : public BWindow {
public:
	KiruWindow();
	~KiruWindow();

	void DispatchMessage(BMessage* message, BHandler* handler) override;
	void MessageReceived(BMessage* message) override;
	bool QuitRequested() override;

private:
	void OpenFile(const char* path);
	void Seek(bigtime_t position, bool precise = false);
	void TogglePlayback();
	void SetMark(bool isInMark);
	void Chop();
	void Tick();
	void ShowAbout();
	void ShowCutFinished(const char* output);
	bool VLCIsInstalled() const;
	status_t OpenInPlayer(const char* output);
	status_t OpenWithApplication(const char* output, const char* signature);
	status_t ShowInTracker(const char* output);
	void UpdateInterface();
	void SetStatus(const char* text);
	bigtime_t CurrentPosition() const;
	BString TimeText(bigtime_t time) const;

	VideoPlayer fPlayer;
	VideoView* fVideoView;
	BSlider* fTimeline;
	BStringView* fTimeLabel;
	BStringView* fMarksLabel;
	BStringView* fStatusLabel;
	BButton* fPlayButton;
	BButton* fCutButton;
	std::unique_ptr<BFilePanel> fOpenPanel;
	std::unique_ptr<BMessageRunner> fTicker;
	bigtime_t fInPoint;
	bigtime_t fOutPoint;
	bigtime_t fScrubPosition;
	bool fPlaying;
	bool fCutting;
	bool fScrubbing;
};

#endif
