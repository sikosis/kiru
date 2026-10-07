#ifndef KIRU_VIDEO_PLAYER_H
#define KIRU_VIDEO_PLAYER_H

#include <MediaFile.h>
#include <String.h>

#include <memory>
#include <vector>

class BBitmap;
class BMediaTrack;

class VideoPlayer {
public:
	VideoPlayer();
	~VideoPlayer();

	status_t Open(const char* path, BString& error);
	status_t Seek(bigtime_t position, bool precise = true);
	status_t ReadFrame();

	BBitmap* Bitmap() const;
	bigtime_t Duration() const;
	bigtime_t Position() const;
	float FrameRate() const;
	bool IsOpen() const;
	const BString& Path() const;

private:
	void Close();

	std::unique_ptr<BMediaFile> fMediaFile;
	BMediaTrack* fVideoTrack;
	std::unique_ptr<BBitmap> fBitmap;
	BString fPath;
	bigtime_t fDuration;
	bigtime_t fPosition;
	bigtime_t fDecodedPosition;
	float fFrameRate;
	int32 fDecodedBytesPerRow;
	int32 fFrameHeight;
	std::vector<uint8> fFrameBuffer;
};

#endif
