zathura - a document viewer
===========================

> **This is a fork with PDF annotations.** It adds Firefox-style text
> highlighting and a freehand pen to zathura, and saves both into the PDF so
> other viewers show them. It needs the matching plugin fork:
> [simanga-dev/zathura-pdf-mupdf](https://github.com/simanga-dev/zathura-pdf-mupdf)
> (branch `annotations`). See [What this fork changes](#what-this-fork-changes).

What this fork changes
----------------------

### Using it

| Key / command | What it does |
|---|---|
| `v v`, `V` or mouse drag | Select text (the first `v` only places the caret) |
| `A` / `:highlight [color]` | Turn the selection into a highlight annotation |
| `X` / `:unhighlight` | Remove the highlights the selection touches |
| `I` / `:pen [color] [width]` | Pen mode: drag with the left button to draw, a click makes a dot |
| `E` / `:eraser` | Eraser mode: click or drag over a stroke to remove it |
| `U` | Undo the last stroke |
| `Esc` | Leave pen or eraser mode |
| `:write` | Save the annotations into the open file (incrementally); `:write other.pdf` saves a copy |

New settings:

    set highlight-annotation-color "#ffff98"
    set pen-color "#e53935"                  # rgba(255,230,0,0.4) gives a see-through highlighter
    set pen-width 2                          # points

Quitting with unsaved highlights or strokes warns once; quitting again
discards them. A hex color can be written without `#` (girara treats `#` as a
comment), e.g. `:pen 0000ff`.

### How it works

* **Plugin API** (`zathura/plugin-api.h`, ABI bumped to 12): four new
  optional plugin functions, `page_add_highlight`, `page_remove_highlights`,
  `page_add_ink` and `page_remove_ink`, with wrappers in `zathura/page.c`.
  Plugins built for the regular zathura do not load into this build, and the
  other way around.
* **Highlights** (`zathura/visual.c`): `zathura_visual_highlight()` collects
  the selection rectangles per page, from the keyboard visual mode, a
  multi-page mouse selection or a page's own mouse selection, and hands them
  to the plugin. The page is re-rendered right away.
* **Pen** (`zathura/draw.c`, `zathura/page-widget.c`): in pen mode the left
  button no longer selects text. The page widget records the pointer path,
  draws it live with cairo, and on release converts it to page coordinates
  (any zoom or rotation) and adds it as an ink annotation. The eraser hits
  strokes within a few pixels of the pointer. Undo removes the newest stroke
  of a page; it forgets a page once the eraser was used on it, so it can never
  remove a stroke that was already in the file.
* **Saving** (`zathura/commands.c`, `zathura/zathura.c`): `:write` with no
  argument saves to the open file. The plugin writes changes incrementally
  where it can.
* **Keys and settings** are registered in `zathura/config.c`; `sc_highlight`
  and `sc_draw` in `zathura/shortcuts.c`; the quit guard is in `sc_quit`.
* **Tests**: `tests/drive_highlight.c` drives a real zathura session headless
  (e.g. with the GTK Broadway backend), feeding keys, commands and pen strokes:

      drive_highlight <plugindir> file.pdf  v v e A  @72,700,150,650  :write

### Installing on Arch

`contrib/arch/PKGBUILD` builds both projects as `zathura-annot` and
`zathura-pdf-mupdf-annot`, which replace `zathura` and `zathura-pdf-mupdf`.
Put the PKGBUILD in a directory next to clones of both forks (directory names
`zathura` and `zathura-pdf-mupdf`), then run `makepkg -si`. The PKGBUILD
expects the regular man pages in a `man/` directory beside it (copy them from
`/usr/share/man`), or remove those lines.

The original zathura README follows.


zathura is a highly customizable and functional document viewer based on the
girara user interface library and several document libraries.

Requirements
------------

The following dependencies are required:

* `gtk4` (>= 4.18)
* `glib` (>= 2.84)
* `girara` (>= 2026.07.07)
* `libmagic` from file(1): for mime-type detection
* `json-glib`
* `sqlite3` (>= 3.25.0): sqlite3 database backend
* `libxxhash`: file hashing

The following dependencies are optional:
* `libsynctex` from TeXLive (>= 2): SyncTeX support
* `libseccomp`: sandbox support

For building zathura, the following dependencies are also required:

* `meson` (>= 1.5)
* `gettext`
* `pkgconf`

The following dependencies are optional build-time only dependencies:

* `librvsg-bin`: PNG icons
* `Sphinx`: manpages and HTML documentation
* `doxygen`: HTML documentation
* `breathe`: for HTML documentation
* `sphinx_rtd_theme`: for HTML documentation

Note that `Sphinx` is needed to build the manpages. If it is not installed, the
man pages won't be built. For building the HTML documentation, `doxygen`,
`breathe` and `sphinx_rtd_theme` are needed in addition to `Sphinx`.

The use of `libseccomp` and/or `landlock` to create a sandboxed environment is
optional and can be disabled by configure the build system with
`-Dseccomp=disabled` and `-Dlandlock=disabled`. The sandboxed version of zathura
will be built into a separate binary named `zathura-sandbox`.  Strict sandbox
mode will reduce the available functionality of zathura and provide a read only
document viewer.

Installation
------------

To build and install zathura using meson's ninja backend:

    meson build
    cd build
    ninja
    ninja install

> **Note:** The default backend for meson might vary based on the platform. Please
refer to the meson documentation for platform specific dependencies.

Bugs
----

Please report bugs at https://github.com/pwmt/zathura.
