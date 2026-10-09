# Rough.js → Field Mouse

This is **a translation of Rough.js drawing algorithms into native Field Mouse**.
It is not a JavaScript wrapper, subprocess to Node, or a new implementation of
one particular Swap picture. The engine returns Rough.js-style **drawing
operations** (path operation sets); SVG generation is a separate adapter.
The small JavaScript reference remains independently readable.

Source: [Rough.js @ 56a2762](https://github.com/rough-stuff/rough/tree/56a2762171b1294d643501e8d14f120db6b27bd7), in particular
`src/renderer.ts`, `src/math.ts` and `src/generator.ts` (MIT; notice in
[`../LICENSE.roughjs`](../LICENSE.roughjs)). The reference generator is
[`../roughjs/line-reference.mjs`](../roughjs/line-reference.mjs).

## Scope of this first port

| Rough.js operation | Field Mouse function | Status |
| --- | --- | --- |
| seeded `Random.next` | `roughRandom(options)` | Exact 31-bit masked integer recurrence using Double-safe arithmetic |
| bowed double-stroke line | `roughLine(x1, y1, x2, y2, options)` | Direct translation of `_line` and `_doubleLine` |
| linear path / polygon | `roughLinearPath(points, options)`, `roughPolygon(points, options)` | Segments preserve shared PRNG state |
| rectangle | `roughRectangle(x, y, w, h, options)` | Four-edge polygon |
| curves | `roughCurve(points, options)` | Endpoint duplication and Catmull-Rom-to-cubic conversion |
| ellipse / circle | `roughEllipse(cx, cy, w, h, options)`, `roughCircle(cx, cy, diameter, options)` | Sampled cubic curves; trigonometry approximated in Field Mouse |
| solid polygon/ellipse fill | `options.fill`, `options.fillStyle = "solid"` | Supported |
| SVG output | `roughSvgDocument(drawables, w, h)` | Separate serializer; no DOM dependency |

Options come from `roughOptions(seed)`, then properties may be changed.
All numeric state uses Field Mouse's Double values; our seeded PRNG works with
integer seeds 1–2147483647. Unlike Rough.js's seed 0, **there is no unseeded
Math.random() mode** yet.

This is **not** the full Rough.js API. Hachure and other pattern fills,
`arc`, arbitrary SVG path parsing, multi-subpath curves, path simplification,
canvas rendering and the remaining style switches are not ported. Hachure
requests are rejected explicitly, not quietly drawn as solid fills. Ellipse
trigonometry uses a Taylor approximation rather than native `Math.sin/cos`,
so expect small numeric differences. Work on this port does not select it as
Swap's final renderer.

## Running the actual Field Mouse interpreter

Field Mouse already has an Idriç interpreter/executor in
[`dilapidated-shed/fieldmouse`](https://github.com/dilapidated-shed/fieldmouse).
Build that executable from its own repository with its documented matching
Idriç/Chez compiler and runtime:

```sh
idris2 --build fieldmouse.ipkg
```

Then, from the Swap repository root:

```sh
FIELD_MOUSE=/absolute/path/to/fieldmouse/build/exec/fieldmouse \
  bash reference-code/fieldmouse/run.sh swap-rough.svg

FIELD_MOUSE=/absolute/path/to/fieldmouse/build/exec/fieldmouse \
  bash reference-code/fieldmouse/run.sh --test
```

`FIELD_MOUSE_REPO=/path/to/fieldmouse` also works if its
`build/exec/fieldmouse` is built, as does a `fieldmouse` on PATH.
The launcher combines `Rough.fm`, `Ellipse.fm`, and the selected script into
a temporary source file and invokes that real Field Mouse runtime directly.
Use `--script my-file.fm [arguments]` for another Field Mouse caller. It
propagates interpreter errors and exit statuses. No JavaScript engine is used
by the port or launcher.

## Differential line acceptance

```sh
FIELD_MOUSE=/path/to/fieldmouse \
  bash reference-code/fieldmouse/run.sh --parity /tmp/rough-ops.json
node reference-code/roughjs/check-parity.mjs /tmp/rough-ops.json
```

The independent JavaScript baseline uses upstream Rough.js's
`Math.imul(48271, state) & 0x7fffffff` random sequence and line formulas.
The checker compares operation types and coordinates to a tolerance of
`1e-7`. The JavaScript fixture is only a **test oracle**; it is never the
interpreter or drawing implementation. The native `--test` fixture also checks
seeded random state, repeatability, stroke counts, curve/circle generation,
solid fill, and SVG production.

## Field Mouse language constraints

Field Mouse uses `←` for assignment and `=` for comparison. `≟` is strict
equality, `≠` is inequality, and `Ø` is false. It has functions, mutable
arrays and objects, but no `for` loops, `Math` object, modulo, bitwise
operations or imports. That is why the PRNG is expressed as 16 binary
subtractions; square root, sine and cosine are expressed explicitly; and the
launcher joins plain source files. None of this requires expanding Field
Mouse's host authority beyond its existing `writeText` operation.

The next substantive porting problems are polygon scanline hachure,
pattern fills, arcs, SVG-path normalization and independent parity fixtures
against Rough.js for each family of primitives.
