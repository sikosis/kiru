#!/bin/sh
set -eu

required="README.md Makefile resources/Kiru.png resources/Kiru-about.rgba resources/Kiru.rdef resources/Kiru.svg src/KiruApp.cpp src/KiruWindow.cpp src/AboutWindow.cpp src/VideoPlayer.cpp src/VideoCutter.cpp"
for file in $required; do
	test -s "$file"
done

for key in MSG_MARK_IN MSG_MARK_OUT MSG_TOGGLE_PLAY MSG_CUT; do
	rg -q "$key" src
done

rg -Fq 'SetModificationMessage(new BMessage(MSG_SCRUB))' src/KiruWindow.cpp
rg -Fq 'fPlayer.Seek(position, precise)' src/KiruWindow.cpp
rg -Fq 'fVideoTrack->CurrentFrame() >= totalFrames' src/VideoPlayer.cpp
rg -Fq 'KiruWindow::DispatchMessage(BMessage* message, BHandler* handler)' src/KiruWindow.cpp
rg -Fq "case 'i':" src/KiruWindow.cpp
rg -Fq "case 'o':" src/KiruWindow.cpp
rg -Fq 'Open in MediaPlayer' src/KiruWindow.cpp
rg -Fq 'Show in Tracker' src/KiruWindow.cpp
rg -Fq 'application/x-vnd.Haiku-MediaPlayer' src/KiruWindow.cpp
rg -Fq 'application/x-vnd.Be-TRAK' src/KiruWindow.cpp
rg -q 'execlp\("ffmpeg"' src/VideoCutter.cpp
rg -q 'resource app_signature "application/x-vnd.sikosis-kiru"' resources/Kiru.rdef
rg -q 'BEOS:L:STD_ICON' resources/Kiru.rdef
rg -q 'BEOS:M:STD_ICON' resources/Kiru.rdef
rg -q 'KIRU:ABOUT_ICON' resources/Kiru.rdef
rg -Fq 'import "Kiru-about.rgba"' resources/Kiru.rdef
rg -q 'B_NOT_RESIZABLE' src/AboutWindow.cpp
if rg -q 'resource vector_icon|BIconUtils::GetVectorIcon' resources/Kiru.rdef src/AboutWindow.cpp; then
	echo "Invalid HVIF runtime path found" >&2
	exit 1
fi
rg -Fq "LoadResource('KICO', 102" src/AboutWindow.cpp
rg -Fq 'B_LARGE_ICON_TYPE, 101' src/AboutWindow.cpp
rg -Fq 'B_TRANSPARENT_MAGIC_CMAP8' src/AboutWindow.cpp
rg -q 'Designed by Sikosis' src/AboutWindow.cpp
rg -q 'Creation Date: 4 October 2026' src/AboutWindow.cpp

if rg -n '^\s*// TODO|^\s*// FIXME' src; then
	echo "Unresolved TODO or FIXME found" >&2
	exit 1
fi

echo "Kiru smoke checks passed."
