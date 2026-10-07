#include "VideoPlayer.h"

#include <Bitmap.h>
#include <Entry.h>
#include <MediaTrack.h>
#include <Path.h>

#include <algorithm>
#include <cstring>

VideoPlayer::VideoPlayer()
	:
	fVideoTrack(nullptr),
	fDuration(0),
	fPosition(0),
	fDecodedPosition(0),
	fFrameRate(25.0f),
	fDecodedBytesPerRow(0),
	fFrameHeight(0)
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


VideoPlayer::~VideoPlayer()
{
	Close();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
VideoPlayer::Close()
{
	fBitmap.reset();
	if (fMediaFile != nullptr && fVideoTrack != nullptr)
		fMediaFile->ReleaseTrack(fVideoTrack);
	fVideoTrack = nullptr;
	fMediaFile.reset();
	fPath.Truncate(0);
	fDuration = 0;
	fPosition = 0;
	fDecodedPosition = 0;
	fDecodedBytesPerRow = 0;
	fFrameHeight = 0;
	fFrameBuffer.clear();
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
VideoPlayer::Open(const char* path, BString& error)
{
	Close();
	entry_ref reference;
	status_t status = get_ref_for_path(path, &reference);
	if (status != B_OK) {
		error.SetToFormat("Could not open the file (%s).", strerror(status));
		return status;
	}

	fMediaFile = std::make_unique<BMediaFile>(&reference);
	status = fMediaFile->InitCheck();
	if (status != B_OK) {
		error.SetToFormat("Haiku could not read this media file (%s).", strerror(status));
		Close();
		return status;
	}

	for (int32 index = 0; index < fMediaFile->CountTracks(); index++) {
		BMediaTrack* track = fMediaFile->TrackAt(index);
		media_format format = {};
		if (track->EncodedFormat(&format) == B_OK && format.IsVideo()) {
			fVideoTrack = track;
			break;
		}
		fMediaFile->ReleaseTrack(track);
	}

	if (fVideoTrack == nullptr) {
		error = "The file does not contain a video track.";
		Close();
		return B_MEDIA_BAD_FORMAT;
	}

	media_format decodedFormat = {};
	decodedFormat.type = B_MEDIA_RAW_VIDEO;
	decodedFormat.u.raw_video.display.format = B_RGB32;
	status = fVideoTrack->DecodedFormat(&decodedFormat);
	if (status != B_OK) {
		error.SetToFormat("The video decoder could not be started (%s).", strerror(status));
		Close();
		return status;
	}

	const media_raw_video_format& raw = decodedFormat.u.raw_video;
	int32 width = raw.display.line_width;
	int32 height = raw.display.line_count;
	if (width <= 0 || height <= 0) {
		error = "The decoder returned an invalid video size.";
		Close();
		return B_MEDIA_BAD_FORMAT;
	}

	fBitmap = std::make_unique<BBitmap>(BRect(0, 0, width - 1, height - 1), 0,
		B_RGB32);
	status = fBitmap->InitCheck();
	if (status != B_OK) {
		error = "There was not enough memory for the video frame.";
		Close();
		return status;
	}

	fDuration = fVideoTrack->Duration();
	fFrameRate = raw.field_rate > 0 ? raw.field_rate : 25.0f;
	fDecodedBytesPerRow = raw.display.bytes_per_row > 0
		? raw.display.bytes_per_row : width * 4;
	fFrameHeight = height;
	fFrameBuffer.resize(static_cast<size_t>(fDecodedBytesPerRow) * fFrameHeight);
	BEntry entry(&reference);
	BPath resolvedPath;
	if (entry.GetPath(&resolvedPath) == B_OK)
		fPath = resolvedPath.Path();
	else
		fPath = path;
	return Seek(0);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
VideoPlayer::Seek(bigtime_t position, bool precise)
{
	if (fVideoTrack == nullptr)
		return B_NO_INIT;
	bigtime_t frameDuration = std::max<bigtime_t>(1,
		static_cast<bigtime_t>(1000000.0f / fFrameRate));
	bigtime_t endSafetyMargin = std::max<bigtime_t>(frameDuration * 3, 100000);
	bigtime_t latestFramePosition = std::max<bigtime_t>(0,
		fDuration - endSafetyMargin);
	bigtime_t requestedPosition = std::clamp(position, static_cast<bigtime_t>(0),
		latestFramePosition);
	bigtime_t seekPosition = requestedPosition;
	status_t status = fVideoTrack->SeekToTime(&seekPosition,
		B_MEDIA_SEEK_CLOSEST_BACKWARD);
	if (status != B_OK)
		return status;
	fDecodedPosition = seekPosition;

	bigtime_t halfFrame = static_cast<bigtime_t>(500000.0f / fFrameRate);
	int32 maximumFrames = precise
		? std::max<int32>(1, static_cast<int32>(fFrameRate * 12)) : 1;
	for (int32 frame = 0; frame < maximumFrames; frame++) {
		status = ReadFrame();
		if (status != B_OK || fDecodedPosition + halfFrame >= requestedPosition)
			break;
	}
	if (!precise && status == B_OK)
		fPosition = requestedPosition;
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
VideoPlayer::ReadFrame()
{
	if (fVideoTrack == nullptr || fBitmap == nullptr || fFrameBuffer.empty())
		return B_NO_INIT;
	int64 totalFrames = fVideoTrack->CountFrames();
	if (totalFrames > 0 && fVideoTrack->CurrentFrame() >= totalFrames)
		return B_LAST_BUFFER_ERROR;

	int64 frameCount = 1;
	media_header header;
	status_t status = fVideoTrack->ReadFrames(fFrameBuffer.data(), &frameCount, &header);
	if (status == B_OK && frameCount > 0) {
		int32 bytesToCopy = std::min(fDecodedBytesPerRow, fBitmap->BytesPerRow());
		uint8* destination = static_cast<uint8*>(fBitmap->Bits());
		for (int32 row = 0; row < fFrameHeight; row++) {
			memcpy(destination + row * fBitmap->BytesPerRow(),
				fFrameBuffer.data() + row * fDecodedBytesPerRow, bytesToCopy);
		}
		fDecodedPosition = header.start_time;
		fPosition = fDecodedPosition;
	}
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


BBitmap*
VideoPlayer::Bitmap() const
{
	return fBitmap.get();
}
//---------------------------------------------------------------------------------------------------------------------------------//


bigtime_t
VideoPlayer::Duration() const
{
	return fDuration;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bigtime_t
VideoPlayer::Position() const
{
	return fPosition;
}
//---------------------------------------------------------------------------------------------------------------------------------//


float
VideoPlayer::FrameRate() const
{
	return fFrameRate;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
VideoPlayer::IsOpen() const
{
	return fVideoTrack != nullptr;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const BString&
VideoPlayer::Path() const
{
	return fPath;
}
//---------------------------------------------------------------------------------------------------------------------------------//
