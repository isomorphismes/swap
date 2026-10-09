# Qualification is not implied by source presence

This branch contains raw `.idric` source and proof terms, a pinned-compiler
workflow, exact reference fixtures, and negative compilation cases.

The status of any check belongs to its actual GitHub Actions run and source
commit. Do not interpret an in-progress bootstrap as a typecheck or test pass.
Do not treat the previous C/Sokol tests or APK as evidence for this package.

The source-level coverage includes:

- action/state/result-pixel witnesses and an all-actions/all-states law for
  the finite pair/formula examples;
- press/release identity, cancellation, occlusion and moving targets;
- submitted source snapshots and fresh output-frame attribution;
- linear response debts and native resources, with failures retaining work;
- negative programs for false pixels, fixed points, hits, frame/picture/surface
  mismatches, receipt substitution and discarded/duplicated resources.

The full braid recognizer, complete scene renderer, concrete NativeBoundary,
Idriç native lowering, native coordinate normalization and event/frame sequencer
are not implemented by this package. Device tests, visual perceptibility,
latency and physical scanout remain unqualified.
