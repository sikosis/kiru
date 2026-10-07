#ifndef KIRU_VIDEO_CUTTER_H
#define KIRU_VIDEO_CUTTER_H

#include <Messenger.h>
#include <String.h>

class VideoCutter {
public:
	static status_t Start(const BString& source, bigtime_t start, bigtime_t end,
		const BMessenger& target, BString& output, BString& error);

private:
	struct Job;
	static int32 Run(void* data);
	static BString OutputPath(const BString& source);
};

#endif

