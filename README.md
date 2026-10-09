# Swap

Three moving identities, their coloured braid histories, and one equilateral
triangle whose three coloured sides move with them. The application is **Swap**;
the repository is now `isomorphismes/swap`.

## First scene

Three separate small streams open into the braid area. **DRAG OVER** and
**DRAG UNDER** specify whether the grabbed strand goes over or under its
neighbour; move a coloured dot horizontally left or right. The release
height does not secretly invert the gesture. Four explicit left/right
over/under buttons also make crossings, and rapid input is queued while a
prior crossing animates. Here "left/right" names the two adjacent **slot
pairs**, not permanently named colour pairs. "Over" in a crossing button
means the left member of that pair goes over. Colours and the small 1/2/3
marks belong to identities.

The matching triangle really rotates through three dimensions. For each
exchange it turns halfway around the axis through the unexchanged vertex and
the opposite side's midpoint. Its three edge colours are attached to the
object, not reassigned to whichever side is on screen. Both faces are drawn;
edge-on and front/back cues make the turnover visible.

An over-crossing followed by another over-crossing restores the triangle's
resting placement but leaves two crossings in the braid. An inverse crossing
is a different move. The program does not infer braid equality from a matching
triangle or from a crossing count.

Reset, replay and pause/resume are included. The viewport shows the latest
crossings and explicitly marks hidden earlier history; replay visits the full
stored word. At the 128-move storage limit, new moves are refused without
changing existing history. No factorials, formula quizzes or general palette
generator are part of this scene.

## Additional small scenes

The top tabs keep the working **THREE** braid/triangle scene intact and add
three independently testable experiments:

- **TWO:** two coloured strands, one signed crossing generator, independent
  stored history and visible winding. Two same-sign crossings return the
  endpoints but not the braid to its initial class.
- **GRID:** a drawable, tappable 3×3 tic-tac-toe board with X/O turns, win/draw
  detection and reset. No conjectured mathematical relation is imposed.
- **SETS:** two-circle membership Venn view with a 2×2 table, a three-circle
  view with an eight-vertex cube, and higher binary tensor tables displayed
  as 2×2 slices. The UI initially offers up to seven binary axes; the model
  supports up to 63 without allocating exponentially many cells. Atom selection
  is separate from any claim of Bayesian inference or p-adic semantics.

[Small views and tensor indexing](docs/discrete-views.md) states the exact
meaning of the displays and outstanding decisions. The new modes still
require physical-device touch/graphics acceptance.

## Code and sketches

- `src/swap.c`: small independent state and geometry model, using Icky C `×`/`÷`.
- `src/app.c`: existing Sokol drawing and NativeActivity adapter; the C67-tested
  triangle rendering is unchanged on the `idric` branch.
- `src/input.c`: small Android-side pointer normalization / control hit
  boundary with a host test exercising C67 coordinates and every button.
- [Executable Idriç candidates](idric/README.md): braid, triangle geometry,
  typed touch decisions and independent tests. The current NDK APK still
  executes the C adapter/model pending Idriç native lowering.
- [Idriç sketches](docs/idric-sketches.md): indexed moves, braid equivalence,
  endpoint actions and exact/numerical boundaries. These are **design sketches**,
  not compiled Idriç or completed proofs.
- [Mathematical connections](docs/mathematics.md): research notes, not additional
  implemented modes.
- [Android build](docs/android.md): ICK C frontend, NDK platform stages, shared
  packaging, immutable dependencies and honest evidence boundaries.

## Checks

With the pinned, qualified host ICK compiler selected as `ICK_CC`:

```sh
cmake -S . -B build/host -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/host
ctest --test-dir build/host --output-on-failure
```

The maintained workflow obtains that compiler through the existing shared
`ai-ci/ick-host` action. It separately builds ARM32 and ARM64 Android libraries.
A host geometry result is not a GPU, APK or physical-device result. The new
C67 control test APK uses package `org.isomorphismes.swap.controls` and can
coexist with the first working Swap installation; its new touch behavior still
requires a physical-device check.

## Rough.js reference and Field Mouse port

The early drawing research keeps an independent
[Rough.js browser demonstration](reference-code/roughjs/browser-demo.html)
and a [native Field Mouse translation of the Rough.js generator](reference-code/fieldmouse/README.md).
The translation includes seeded rough strokes, curves, polygons, rectangles,
circles/ellipses, SVG serialization, a launcher using the actual
`dilapidated-shed/fieldmouse` interpreter, and differential reference tests.

These are reference implementations, **not** a commitment to using Rough.js
or the Field Mouse port as Swap's production Android renderer.
