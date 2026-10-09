<p align="center">
  <img src="resources/Kiru.png" alt="Kiru axe and film icon" width="256">
</p>

<h1 align="center">Kiru</h1>

<p align="center">
  <img src="https://img.shields.io/badge/version-v0.19-f5a623" alt="Version v0.19">
  <img src="https://img.shields.io/badge/platform-Haiku-ffcc00" alt="Platform: Haiku">
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599c" alt="C++17">
  <img src="https://img.shields.io/badge/licence-MIT-4c8c2b" alt="MIT licence">
</p>

Kiru (Japanese for “cut”) is a small, keyboard-first video chopping tool for
Haiku. Drop a video onto the window, mark an in and out point, preview the
selection, then write it as a new file without re-encoding.

## Controls

| Key | Action |
| --- | --- |
| Timeline | Drag for a responsive preview position; release to decode that frame |
| `L` | Load a video using the file panel |
| `I` | Set the in point |
| `O` | Set the out point |
| `Space` or `P` | Play/pause; with both marks set, preview only the selection |
| `K` | Chop the marked selection into a new video |
| `Left` / `Right` | Seek backward/forward one second |
| `Shift+Left` / `Shift+Right` | Seek backward/forward five seconds |
| `J` | Seek backward five seconds |
| `,` / `.` | Step backward/forward one video frame |
| `Home` / `End` | Seek to the beginning/end |

Videos can also be opened with the Load button, by dropping a file on the
window, or by passing a path on the command line.

After a successful chop, Kiru can open the new clip in MediaPlayer or open its
containing folder in Tracker. If VLC is installed, Kiru detects its Haiku
application signature and also offers it in the player chooser.

Design credits, dependency licences, acknowledgements and project dates can be
found in the About dialog.

## Requirements

- Haiku with the Media Kit development headers
- `ffmpeg` in `PATH` (available from HaikuDepot)
- `make`, `rc`, and `xres`

At startup, Kiru checks whether `ffmpeg` is available in `PATH`. If it is
missing, Kiru can open Terminal and run `pkgman install ffmpeg` after receiving
confirmation from the user.

Kiru uses Haiku's Media Kit for its silent, in-window preview. Cutting uses
FFmpeg's stream-copy mode, so it is quick and does not reduce quality. As with
all stream-copy cutters, the first frame of an output may be moved to the
nearest usable keyframe.

## Build

```sh
make
```

The resulting `Kiru` binary is in the project directory. Install it for the
current user with:

```sh
make install
```

## Smoke checks

```sh
make check
```

The smoke checks validate source/resource wiring and the repository's style
rules. A full build must be performed on Haiku because the Media Kit is not
available on other operating systems.

## Credits and licences

Kiru was designed by Sikosis. It uses Haiku's Application, Interface, Media,
and Tracker Kits, distributed under the MIT licence. Fast cutting is performed
by the separately installed FFmpeg executable, distributed under the LGPL or
GPL depending on its build configuration. VLC is supported as an optional
external player and is distributed under the GPL. Kiru's original axe-and-film
icon is inspired by the dimensional application icons of BeOS and Haiku.
