# Swap

Three moving identities, their coloured braid histories, and one equilateral
triangle whose three coloured sides move with them. The application is **Swap**;
the repository currently remains `isomorphismes/switch`.

## First scene

Three separate small streams open into the braid area. Drag an endpoint to its
neighbour: finish above the handle row to carry the dragged strand over, or
below to carry it under. Four buttons also expose left/right, over/under
crossings. Here “left/right” selects one of the two adjacent **positions**, not
a permanently named pair of colours. “Over” in a button means the left member
of that pair goes over. Colours and the small 1/2/3 marks belong to identities.

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

## Code and sketches

- `src/swap.c`: small independent state and geometry model, using Icky C `×`/`÷`.
- `src/app.c`: Sokol drawing and touch interaction; NativeActivity on Android.
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
A host geometry result is not a GPU, APK or physical-device result.

## Rough.js reference and Field Mouse port

The early drawing research keeps an independent
[Rough.js browser demonstration](reference-code/roughjs/browser-demo.html)
and a [native Field Mouse translation of the Rough.js generator](reference-code/fieldmouse/README.md).
The translation includes seeded rough strokes, curves, polygons, rectangles,
circles/ellipses, SVG serialization, a launcher using the actual
`dilapidated-shed/fieldmouse` interpreter, and differential reference tests.

These are reference implementations, **not** a commitment to using Rough.js
or the Field Mouse port as Swap's production Android renderer.
