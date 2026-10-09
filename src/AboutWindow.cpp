#include "AboutWindow.h"

#include <Application.h>
#include <Bitmap.h>
#include <Button.h>
#include <Font.h>
#include <GroupLayoutBuilder.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <Mime.h>
#include <Resources.h>
#include <Roster.h>
#include <SeparatorView.h>
#include <Size.h>
#include <String.h>
#include <StringView.h>
#include <View.h>

#include <cstring>

class IconView : public BView {
public:
	IconView(BBitmap* icon);
	~IconView();

	void Draw(BRect updateRect) override;

private:
	BBitmap* fIcon;
};

class LinkView : public BStringView {
public:
	LinkView(const char* name, const char* label, const char* url);

	void AttachedToWindow() override;
	void MouseDown(BPoint point) override;

private:
	BString fUrl;
};

IconView::IconView(BBitmap* icon)
	:
	BView("application icon", B_WILL_DRAW),
	fIcon(icon)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetExplicitMinSize(BSize(96, 96));
	SetExplicitMaxSize(BSize(96, 96));
}
//---------------------------------------------------------------------------------------------------------------------------------//


IconView::~IconView()
{
	delete fIcon;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
IconView::Draw(BRect updateRect)
{
	(void)updateRect;
	if (fIcon == nullptr)
		return;
	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	DrawBitmap(fIcon, fIcon->Bounds(), Bounds(), B_FILTER_BITMAP_BILINEAR);
}
//---------------------------------------------------------------------------------------------------------------------------------//


LinkView::LinkView(const char* name, const char* label, const char* url)
	:
	BStringView(name, label),
	fUrl(url)
{
	SetToolTip(url);
	BFont font;
	GetFont(&font);
	font.SetFace(font.Face() | B_UNDERSCORE_FACE);
	SetFont(&font);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
LinkView::AttachedToWindow()
{
	BStringView::AttachedToWindow();
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	rgb_color background = ui_color(B_PANEL_BACKGROUND_COLOR);
	int32 luminance = (299 * background.red + 587 * background.green
		+ 114 * background.blue) / 1000;
	if (luminance < 128)
		SetHighColor(105, 190, 255);
	else
		SetHighColor(0, 76, 168);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
LinkView::MouseDown(BPoint point)
{
	(void)point;
	const char* arguments[] = {fUrl.String()};
	be_roster->Launch("text/html", 1, arguments);
}
//---------------------------------------------------------------------------------------------------------------------------------//


AboutWindow::AboutWindow()
	:
	BWindow(BRect(0, 0, 560, 620), "About Kiru", B_TITLED_WINDOW,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS
			| B_CLOSE_ON_ESCAPE)
{
	BBitmap* icon = nullptr;
	BResources* resources = BApplication::AppResources();
	if (resources != nullptr) {
		size_t resourceSize = 0;
		const uint8* colourData = static_cast<const uint8*>(
			resources->LoadResource('KICO', 102, &resourceSize));
		if (colourData != nullptr && resourceSize >= 64 * 64 * 4) {
			icon = new BBitmap(BRect(0, 0, 63, 63), B_RGBA32);
			if (icon->InitCheck() != B_OK) {
				delete icon;
				icon = nullptr;
			} else {
				for (int32 y = 0; y < 64; y++) {
					std::memcpy(static_cast<uint8*>(icon->Bits())
							+ y * icon->BytesPerRow(), colourData + y * 64 * 4,
						64 * 4);
				}
			}
		}
		const uint8* bitmapData = static_cast<const uint8*>(resources->LoadResource(
			B_LARGE_ICON_TYPE, 101, &resourceSize));
		if (icon == nullptr && bitmapData != nullptr) {
			icon = new BBitmap(BRect(0, 0, 31, 31), B_RGBA32);
			if (icon->InitCheck() != B_OK) {
				delete icon;
				icon = nullptr;
			} else if (resourceSize < static_cast<size_t>(32 * 32)) {
				delete icon;
				icon = nullptr;
			} else {
				std::memset(icon->Bits(), 0, static_cast<size_t>(icon->BitsLength()));
				for (int32 y = 0; y < 32; y++) {
					uint32* destination = reinterpret_cast<uint32*>(
						static_cast<uint8*>(icon->Bits()) + y * icon->BytesPerRow());
					for (int32 x = 0; x < 32; x++) {
						if (bitmapData[y * 32 + x] != B_TRANSPARENT_MAGIC_CMAP8)
							destination[x] = 0xff000000;
					}
				}
			}
		}
	}

	BStringView* title = new BStringView("title", "Kiru");
	BFont titleFont(*be_bold_font);
	titleFont.SetSize(be_plain_font->Size() * 2.0f);
	title->SetFont(&titleFont);
	BStringView* version = new BStringView("version", "Version v0.19");
	BStringView* description = new BStringView("description",
		"A quick, keyboard-first video chopping tool for Haiku.");

	BStringView* projectHeading = new BStringView("project heading", "Project");
	projectHeading->SetFont(be_bold_font);
	BStringView* licenceHeading = new BStringView("licence heading",
		"Licences and dependencies");
	licenceHeading->SetFont(be_bold_font);
	BStringView* thanksHeading = new BStringView("thanks heading", "Special thanks");
	thanksHeading->SetFont(be_bold_font);

	BButton* closeButton = new BButton("close", "Close", new BMessage(B_QUIT_REQUESTED));
	closeButton->MakeDefault(true);

	SetLayout(new BGroupLayout(B_VERTICAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 8.0f)
		.SetInsets(18.0f, 18.0f, 18.0f, 18.0f)
		.AddGroup(B_HORIZONTAL, 16.0f)
			.Add(new IconView(icon))
			.AddGroup(B_VERTICAL, 4.0f)
				.Add(title)
				.Add(version)
				.AddStrut(4.0f)
				.Add(description)
				.AddGlue()
			.End()
		.End()
		.Add(new BSeparatorView(B_HORIZONTAL))
		.Add(projectHeading)
		.Add(new BStringView("designer", "Designed by Sikosis"))
		.Add(new BStringView("created", "Creation Date: 4 October 2026"))
		.Add(new BStringView("updated", "Updated: 10 October 2026"))
		.AddStrut(5.0f)
		.Add(licenceHeading)
		.Add(new BStringView("haiku licence",
			"Haiku Application, Interface, Media and Tracker Kits — MIT licence"))
		.Add(new LinkView("haiku link", "https://www.haiku-os.org/",
			"https://www.haiku-os.org/"))
		.Add(new BStringView("ffmpeg licence",
			"FFmpeg — external dependency; LGPL/GPL depending on build"))
		.Add(new LinkView("ffmpeg link", "https://ffmpeg.org/", "https://ffmpeg.org/"))
		.Add(new BStringView("vlc licence",
			"VLC media player — optional external player; GPL licence"))
		.Add(new LinkView("vlc link", "https://www.videolan.org/vlc/",
			"https://www.videolan.org/vlc/"))
		.Add(new BStringView("icon credit",
			"Original BeOS-inspired axe-and-film icon"))
		.AddStrut(5.0f)
		.Add(thanksHeading)
		.Add(new BStringView("thanks",
			"The Haiku, FFmpeg and VideoLAN communities."))
		.AddGroup(B_HORIZONTAL, 0)
			.AddGlue()
			.Add(closeButton)
		.End());

	CenterOnScreen();
}
//---------------------------------------------------------------------------------------------------------------------------------//
