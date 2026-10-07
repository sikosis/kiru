#ifndef KIRU_VIDEO_VIEW_H
#define KIRU_VIDEO_VIEW_H

#include <View.h>

class BBitmap;

class VideoView : public BView {
public:
	VideoView();

	void Draw(BRect updateRect) override;
	void SetBitmap(BBitmap* bitmap);

private:
	BBitmap* fBitmap;
};

#endif

