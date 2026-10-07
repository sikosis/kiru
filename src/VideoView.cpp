#include "VideoView.h"

#include <Bitmap.h>

#include <algorithm>

VideoView::VideoView()
	:
	BView("video", B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	fBitmap(nullptr)
{
	SetViewColor(B_TRANSPARENT_COLOR);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
VideoView::Draw(BRect updateRect)
{
	(void)updateRect;
	SetHighColor(18, 20, 24);
	FillRect(Bounds());

	if (fBitmap == nullptr)
		return;

	BRect source = fBitmap->Bounds();
	BRect destination = Bounds();
	float scale = std::min(destination.Width() / source.Width(),
		destination.Height() / source.Height());
	float width = source.Width() * scale;
	float height = source.Height() * scale;
	destination.left += (destination.Width() - width) / 2.0f;
	destination.top += (destination.Height() - height) / 2.0f;
	destination.right = destination.left + width;
	destination.bottom = destination.top + height;
	DrawBitmap(fBitmap, source, destination, B_FILTER_BITMAP_BILINEAR);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
VideoView::SetBitmap(BBitmap* bitmap)
{
	fBitmap = bitmap;
	Invalidate();
}
//---------------------------------------------------------------------------------------------------------------------------------//
