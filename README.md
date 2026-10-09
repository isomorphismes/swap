# Swap — raw Idriç branch

This branch develops the relationship between touching a painted object,
applying its operation, and displaying the result. The active source is
[`raw-idric/`](raw-idric/README.md), not the earlier C/Sokol prototype.

The central contract is stronger than a named gesture or a redraw request:
if the operation changes the current state, the resulting picture must have
an actual changed pixel in the operation's result region. The proof is indexed
by the operation and before-state. A fixed-point proof justifies a semantic
no-change result; an invisible state transition is refused, not called success.

The direct Android boundary is NDK input / NativeActivity / EGL / GLES.
The new package neither imports Sokol nor copies its application logic into C.
The former native workflow is removed on this branch so it cannot build a
Sokol APK and label that as raw Idriç acceptance.

## Source and status

- [Screen and response types](raw-idric/README.md): coupled paint/picking,
  operation semantics, result-region and changed-pixel proofs.
- [Native boundary](raw-idric/Swap/Raw/Native.idric): surface/frame/input indices,
  linear resources, verified pixels and distinct buffer-submission results.
- [Tests](raw-idric/Swap/Raw/Tests.idric): small exact reference-raster cases and
  compiler-rejected negative programs. The pinned compiler must actually run
  before any typecheck result is claimed.

A concrete Idriç-to-NDK graphics implementation and full Swap UI are not yet
provided by this branch. No APK or physical-device result is claimed.

The inherited `src/`, `idric/`, `android/`, and legacy build scripts remain
available for comparison. They are earlier prototypes, not alternate raw build
routes. The other branches and the previously installed Swap are unchanged.
The Rough.js and Field Mouse material under `reference-code/` remains reference
material, not a dependency of this work.
