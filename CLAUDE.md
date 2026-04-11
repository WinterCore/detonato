# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Detonato is a stopwatch/counter and pomodoro timer rendered with OpenGL, using Posy's 7-segment digit shapes. Written in C, depends on GLFW3 + GL. Vendored deps: glad (GL loader), libtess2 (polygon tessellation), nanosvg (SVG parser, header-only).

## Build & Run

```sh
make              # builds both ./detonato and ./preprocess
./detonato        # must be run from repo root — asset/shader paths are relative
make clean
```

The Makefile builds **two binaries from overlapping source sets**, not one:

- **`detonato`** — the runtime. Loads a baked `.bin` style and runs the timer. Does *not* compile `preprocess.c` or libtess2.
- **`preprocess`** — the baker. Loads a `.svg` from `assets/`, tessellates it, renders it so you can eyeball the result, and writes a `.bin` to `styles/` when you confirm. Compiles `preprocess.c` + libtess2 *and* the GL stack (so it can render for visual verification).

Both link against GLFW/GL because `preprocess` needs the renderer to preview styles before baking.

Debug build is always on: `CFLAGS` includes `-g -O0 -Wall -Wextra`. Run under gdb with `gdb ./detonato`. There is no test suite, linter, or formatter. `compile_commands.json` is checked in for clangd.

## Architecture

Two-phase design: a **bake step** (preprocess SVG → binary) and a **runtime** (load binary → render). The two phases share the `DigitStyle` struct as the handoff format.

### `DigitStyle` — the central data structure

Defined in `src/style.h`. Contains everything needed to render one 7-segment typeface:

- `aspect_ratio` — digit width/height, driven by the source SVG viewbox
- `segment_count` — actual number of segments for this style (styles have between 5 and 35, max `MAX_SEGMENTS = 64`)
- `segment_bitmask[10]` — one `uint64_t` per decimal digit, bit `i` set = segment `i` is lit for that digit. Hand-authored per style.
- `segment_vertex_start[MAX_SEGMENTS]`, `segment_vertex_count[MAX_SEGMENTS]` — slice offsets into the flat vertex buffer, per segment
- `vertices` (float *) — flat `[x,y, x,y, ...]` of all segments concatenated, normalized to unit space `[0,1]`, ready for `glDrawArrays(GL_TRIANGLES, start, count)`

**Why `vertices` is a pointer** (not a flexible array member): the serialization code in `style.c` fwrites/freads fields explicitly rather than dumping the struct as a contiguous blob, so the pointer is allocated separately. Deliberate trade — more code, fewer footguns when the schema changes.

### Baking pipeline (`preprocess` binary)

`src/preprocess.c` owns the SVG → `DigitStyle` conversion:

1. **nanosvg** parses the file into `NSVGshape` linked list — one shape per physical segment piece.
2. Each shape's cubic Bézier paths are flattened to polylines (30 subdivisions per segment) by `flatten_cubic_bez`. `skip_first` avoids duplicating the join vertex between consecutive Béziers.
3. Each contour is fed to **libtess2**, which emits indexed triangles respecting the SVG fill rule (even-odd vs non-zero).
4. Indices are expanded to flat `[x,y,...]` per-segment arrays stored in `DigitSegment` (flexible array member, one per SVG shape) inside a `SegmentDigitShape` intermediate.
5. `build_segment_mesh` concatenates all segments' vertices into a single flat buffer and populates `segment_vertex_start` / `segment_vertex_count`. Vertices are normalized to `[0,1]` using the SVG viewbox.

The `preprocess` executable then renders the resulting `DigitStyle` in a window so a human can visually confirm the geometry before `write_digit_style` (in `src/style.c`) serializes it to `styles/<name>.bin`. The `segment_bitmask[10]` table is authored by hand per style — bit index corresponds to SVG path order, so reordering paths in Inkscape silently invalidates the map.

**Important:** the SVG shape order defines the segment index (0..N-1). Any change to an SVG must preserve that ordering or the hand-authored `segment_bitmask` breaks.

### Runtime (`detonato` binary)

`src/main.c` loads a `DigitStyle` from a `.bin`, uploads the flat vertex buffer once, and draws segments per frame. For each lit segment of each digit in the timer, the main loop calls `draw_range(mesh, segment_vertex_start[i], segment_vertex_count[i])`. Digit selection is driven by `segment_bitmask[digit]` — iterate bits 0..segment_count, draw if set.

Horizontal layout is handled by `compute_layout` in `src/render.c`. It takes a pattern string like `"DD:DD:DD.DDD"` where `D` is a digit slot and `:` / `.` are narrow separator slots (configurable width via `narrow_ratio`). It returns a `DigitLayout` containing per-slot `{x, w, kind}` plus global `digit_w` / `digit_h` / `narrow_w`. The layout function is horizontal-only; the caller computes `y` for vertical centering from `digit_h` and the viewport height.

The vertex shader (`shaders/segment.vert`) takes unit-space positions and transforms them via `offset` + `scale` uniforms (in pixels), then converts to NDC using `viewWidth` / `viewHeight`. The fragment shader is a flat `color + alpha` output. **Blending is not enabled**, so alpha is effectively ignored until `glEnable(GL_BLEND)` is added.

### Supporting files

- `src/gl_helpers.{c,h}` — GLFW window creation, shader compile/link, VAO/VBO setup, `draw_range`. Thin wrapper over the GL API.
- `src/render.{c,h}` — `compute_layout` and `DigitLayout`. Pure layout math, no GL calls.
- `src/aids.{c,h}` — utility macros (`PANIC`, `UNIMPLEMENTED`, `UNREACHABLE`, `DEBUG_PRINTF`, `MIN`/`MAX`) and math helpers (`clamp`, `lerp`, `remap`). `PANIC` is variadic printf-style with `__FILE__:__LINE__`; `UNIMPLEMENTED` / `UNREACHABLE` are older and not `do { } while(0)`-wrapped (they'll break inside `if/else` without braces).
- `src/style.{c,h}` — `DigitStyle` struct + `write_digit_style` (binary serializer). Matching reader will live here too.

## Known issues / tech debt

- **Separators (colons, dots) are unimplemented.** `compute_layout` emits narrow slots for them but `main.c` doesn't draw anything in those slots. Current plan: bake separator shapes into each SVG as additional paths, or draw squares as a placeholder.
- **Main loop has a 7-hardcoded draw loop** (`for (int i = 0; i < 7; i++)`) and a shadowed `i` variable inside the layout loop. Will need updating when digit selection via `segment_bitmask` is wired up.
- **`main.c` still constructs `SegmentMeshData`** via `build_segment_mesh` directly from a freshly-parsed SVG — it does not yet load `.bin` files. The runtime/bake split is structural but not complete.
- **Long-term: `detonato` should drop the libtess2/nanosvg dependency entirely** once it only loads `.bin` files. Right now both binaries include the full parsing stack.
- **No timer/pomodoro state machine yet.** Time tracking, mode switching, and multi-digit layout for actual time values are all unimplemented. Only a static pattern is drawn.
