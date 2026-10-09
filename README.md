# Swap — raw Idriç branch

The active source is [`raw-idric/`](raw-idric/README.md), not the earlier
C/Sokol prototype. This branch connects the painted object you touch to its
operation and to the exact result that must be drawn through a direct
Android NDK/EGL/GLES boundary.

## Central relationships

The application entry requires [`Responsive behaviour`](raw-idric/responsiveness-law.md):
a total witness that every action which changes its current state also changes
an actual pixel in the action's meaningful result region. The witness is bound
to `draw (perform command before)`, not an arbitrary replacement picture.
A fixed-point proof permits no semantic change.

[Submitted source snapshots](raw-idric/native-frame-link.md) tie press/release
pictures to native frame receipts. A release's own event serial is distinct
from the input that produced its observed frame. Its response needs a newer
frame key attributed to that release.

The application-facing response is an opaque linear obligation. It must be
submitted as that exact result, justified as a fixed point, or retained on
failure. It cannot be unwrapped merely to discard a redraw request.

[Detailed contracts](raw-idric/contracts.md) state coordinate, capture, action,
queue, animation, native readback/submission, lifecycle and progress obligations.
Positive reference-raster cases and negative compiler cases accompany the code.

## Evidence and boundaries

The exact `.idric` source must pass the pinned compiler and executable tests
before qualification is claimed. A concrete native driver, full Swap UI,
GPU execution, new APK and physical-screen behavior are not provided by this
contract branch. A typed submission receipt is not proof of physical scanout
or of a human-perceptible change.

No Sokol, SDL, raylib or JavaScript renderer is imported by the raw package.
The former Sokol native workflow is removed only on this branch so it cannot
produce an APK mistaken for raw Idriç acceptance. The inherited `src/`,
`idric/`, `android/` and legacy scripts remain for comparison, not as alternative
raw build routes. Other branches and the installed Swap are unchanged.
Rough.js and Field Mouse under `reference-code/` remain reference material.
