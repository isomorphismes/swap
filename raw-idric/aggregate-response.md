# Correction: visible response is an aggregate

The user had already required aggregate visible response. Replacing this with
'exists one changed pixel', followed by a future perceptibility caveat, was
not an implementation of that requirement. The old constructor is removed.

## Inspected Fourier-sound evidence

Repository: isomorphismes/Fourier-sound, GPU branch at exact commit
`3c2ecaafd9384b2096dd55b11cca5a74ec0c6c83`.

1. `tests/media_fixture_test.c`, `compare_constructions`, uses 128x128 images.
   Both dense and dyadic constructions must be nonconstant over more than 10%
   of addresses (relative to the first colour). Between the two constructions,
   changed addresses must exceed one fifth of the image and mean absolute RGB8
   channel difference must exceed 1.0. These are whole-image aggregates, not a
   one-pixel test. They compare two constructions, NOT a touchscreen event.
   https://github.com/isomorphismes/Fourier-sound/blob/3c2ecaafd9384b2096dd55b11cca5a74ec0c6c83/tests/media_fixture_test.c
2. `tests/render_test.c` traverses PCM -> DFT -> sampled complex field -> Wegert
   RGB -> PPM. More than half of the 33x33 image must differ from the first
   pixel, and every serialized RGB byte is checked. This is a nonconstant-image
   test, not an inter-frame visibility threshold.
   https://github.com/isomorphismes/Fourier-sound/blob/3c2ecaafd9384b2096dd55b11cca5a74ec0c6c83/tests/render_test.c
3. `tests/native_window_output_test.c` checks format, stride, buffer writing,
   lock and post outcomes using a fake native window. It is host boundary
   evidence, NOT a physical handset display result.
   https://github.com/isomorphismes/Fourier-sound/blob/3c2ecaafd9384b2096dd55b11cca5a74ec0c6c83/tests/native_window_output_test.c

No new execution of those Fourier tests is claimed by this source inspection.

## Lift the same structure into the type relationship

For the action's complete, predeclared meaningful result region R:

    N = number of actual raster addresses in R
    K = number whose final RGB differs
    D = sum over R and RGB channels of absolute channel difference

`VisibleChange behaviour command before` requires evidence that the computed
N, K and D meet the policy for that exact behaviour/action/before-state:

    N >= minimum region size
    K >= minimum changed area
    K * fraction denominator > N * fraction numerator
    D > 3 * N * minimum mean RGB8 channel difference

Both area and magnitude are required. One bright pixel can have a large local
contrast and still fail the area conditions. Widespread one-unit channel noise
can have full coverage and still fail the magnitude condition. Empty/singleton
regions and zero/vacuous budgets are rejected. The pure measurement enumerates
`all_pixels canvas` once and clips with the result-region predicate; no caller
can certify success by repeating one sample or passing manufactured statistics.

Region identity and budget belong to the behaviour, not a post-hoc screenshot
crop. A native implementation must use the same actual displayed coordinate
scale and must establish that its verified/submitted pictures match the ones
in this certificate. A simulated or intended drawing is not native evidence.

The current finite fixtures use an actual 8x8 result patch (64 addresses), at
least 16 changed addresses, >20% coverage, and RGB8 mean >1. The 64/16 floors
are NEW arithmetic-regression choices to reject tiny regions; they are not a
prior user agreement or calibrated phone thresholds. The general policy's
nonvacuity checks alone are not calibration either. Real controls and formula
results need appropriate reviewed regions and device-scale aggregate budgets.
Do not use a percentage of the whole phone screen for every small object.

## Regression cases

Positive: all 64 result addresses change correctly, both pair states, formula
activation, reset from a nonfixed state. Fixed-point reset remains exempt.
Negative: a single bright result pixel; the original one-pixel example; a
post-hoc singleton region; flashing only the control; stale result rendering.
Predicate unit cases separately reject widespread one-unit channel noise,
exact threshold equality, and vacuous policies. They are NOT extra GPU tests.

The all-actions law, source-frame association and linear response debt remain.
A changed model must yield this aggregate certificate, not just event delivery,
metadata, a pixel inequality, or acknowledgement. Meaningful duration, an
animation's intermediate frames and latency remain to be integrated; changing
an aggregate for one imperceptibly short frame is not declared phone acceptance.
