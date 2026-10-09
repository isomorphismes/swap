# Swap, Idriç branch

The first real typed-language candidate for Swap lives here:

- `Swap/Model.idric`: three **identities**, three **slots**, adjacent signed
  crossings, separate history versus final permutation, queued rapid moves,
  pause and replay. This keeps braid history even when endpoints coincide.
- `Swap/Geometry.idric`: the same rigid 3D half-turn geometry that the
  first C67 build exhibited, plus sampled 3D crossings.
- `Swap/Input.idric`: named buttons, screen-pixel-to-canvas conversion, grabbed
  identity, explicit over/under selection and pure press/release outcomes.
- `Swap/Tests.idric`: deterministic executable host scenarios for all buttons
  at 576×1152 C67 coordinates, letterboxing, two crossing orientations,
  cancellation, queueing, triangle side lengths and representative algebra.

The proposed program is more than diagrammatic pseudocode; it is authored in
the canonical Idriç source file format (`.idric`) and compiled/tested in a
separate CI job using an exact pinned Idriç checkout. Results are pending until
that job actually completes. Do not interpret a passing ICK/NDK build as
evidence that the Idriç code executed.

## Build and tests

With the pinned Idriç compiler from
`isomorphisms/Idric@94dfd99bd3e376507fedc8611053b7173b2519f0`:

```sh
export IDRIS2_PATH="/path/to/Idric/_/libs/prelude/build/ttc:/path/to/Idric/_/libs/base/build/ttc:/path/to/Idric/_/libs/contrib/build/ttc:/path/to/Idric/_/libs/linear/build/ttc:/path/to/Idric/_/libs/network/build/ttc:/path/to/Idric/_/libs/test/build/ttc"
cd idric
/path/to/Idric/_/build/exec/idris2 --build swap.ipkg
./build/exec/swap-idric-tests
```

The `idric-semantics` job boots that compiler itself and executes these cases,
independently of the native build job. Do not run them through stock C, RefC, or
an unqualified alternate interpreter.

## Honest execution boundary

The screen-tested Android app remains Sokol + ICK C + NDK. The current direct
Idriç DEX/ART lane is limited to checked small integer operations; it cannot
yet lower this model's aggregates, floating geometry, native window, GLES
drawing and touch event lifetimes into the complete C67 application.

Consequently the branch includes a **separate installable native control test**
which implements the same hit/drag rules as `Input.idric` in a small C
boundary (`src/input.c`). Its button coordinates are shared by the native
drawer and native input tests. This makes the UI failures observable and
testable now, without pretending that a new source language repaired a
platform bug by itself.

The new test candidate installs as `org.isomorphismes.swap.controls`
("Swap Controls") alongside the first working Swap. The test build uses an
ephemeral signing key and cannot update a prior build of this test package
without uninstalling that test package first.

Future native Idriç lowering must replace the semantics explicitly, compare
these same input/geometry fixtures, and preserve the Sokol surface rather than
silently falling back to an untyped alternate compiler.
