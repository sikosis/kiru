# Kiru

Kiru (Japanese for “cut”) is a small, keyboard-first video chopping tool for
Haiku. Drop a video onto the window, mark an in and out point, preview the
selection, then write it as a new file without re-encoding.

## Controls

| Key | Action |
| --- | --- |
| Timeline | Drag for a responsive preview position; release to decode that frame |
| `I` | Set the in point |
| `O` | Set the out point |
| `Space` or `P` | Play/pause; with both marks set, preview only the selection |
| `K` | Chop the marked selection into a new video |
| `Left` / `Right` | Seek backward/forward one second |
| `J` / `L` | Seek backward/forward five seconds |
| `,` / `.` | Step backward/forward one video frame |
| `Home` / `End` | Seek to the beginning/end |

Videos can also be opened with the Open button, by dropping a file on the
window, or by passing a path on the command line.

After a successful chop, Kiru can open the new clip in MediaPlayer or open its
containing folder in Tracker.

The Kiru menu includes an About window with version information, project
dates, design credit, dependency licences, and acknowledgements.

## Requirements

- Haiku with the Media Kit development headers
- `ffmpeg` in `PATH` (available from HaikuDepot)
- `make`, `rc`, and `xres`

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
GPL depending on its build configuration. Kiru's original axe-and-film icon is
inspired by the dimensional application icons of BeOS and Haiku.
