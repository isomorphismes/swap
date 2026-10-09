# Relationships from a finger to a visible result

## 1. Pixels, geometry and identity

The reference picture is a total function on bounded `PixelAddress canvas`.
Each final sample owns both its opaque pigment and optional interaction target.
Picking reads that ownership after the same clipping and composition as paint.
An opaque unowned foreground object blocks a background control. Transparent
coverage does not. NDK screen coordinates must be normalized ONCE with the
live viewport's orientation, offset, scale and content insets. Reject NaN,
infinity, out-of-window coordinates and invalid pointer indices before creating
bounded pixel addresses. Different viewport dimensions invalidate old addresses
at the type boundary; equal dimensions but changed transforms require a new
viewport revision at runtime.

The finite reference raster is an executable specification, not the production
rendering algorithm. Exhaustively checking every pixel per touch would be a
poor implementation for the phone. A native renderer should carry certified
changed patches/damage through its scene construction, then exercise the same
reference cases. It must not turn a faster approximate hit box into an
independent second geometry model.

## 2. Press, release and revalidation

`Contact` stores the actual press event and its `Hit` proof. `Released` requires
an end event from the same pointer, surface epoch and viewport revision, a
nondecreasing displayed-frame revision, a strictly later input serial, and a
fresh hit on the same object identity/kind/operation in the current picture.

An object may move between press and release. The current painted shape is
used; the old slot is not a target. An ended event for another pointer or a
cancel event cannot activate the captured object. A vanished/replaced target,
changed action, stale surface or viewport becomes an explicit rejection.

The contact driver must pair each supplied picture with its actual frame key,
and validate monotonicity against its authoritative latest-frame/event state.
The pure core cannot verify a forged event supplied by a dishonest FFI adapter.
A later release serial alone does not prove an event has never been processed.
Deduplication and lease ownership remain required at that boundary.

This implemented contact rule is click activation for buttons/formulas. A braid
DRAG does not require releasing on the originally grabbed object. Its eventual
rule must preserve the captured identity, use current geometry to resolve its
neighbour, bind explicit over/under to the grabbed identity, and then create the
same action/result obligation. The raw branch does not claim to have implemented
that full drag recognizer or the old triangle renderer.

## 3. Action, state, and pixels

`respond_to_release` selects the operation from the validated current target and
applies it to the current model, not the press-time model. A `Response` describes:

- `AtFixedPoint`: a proof that this operation fixes this state;
- `NeedsDrawing`: a state-change proof and a changed result-pixel witness;
- `UnobservableChange`: the state changes but the chosen view does not show it.

`VisibleChange`'s picture is definitionally `draw (perform command before)`.
A caller cannot substitute another next state, a stale frame, or a different
command. Its sample must be inside the operation's declared result region.
This declaration is a semantic obligation: it must point to the formula,
strand, board cell, history or other actual explanation of the result. It must
not be declared to include an unrelated incrementing status counter.

A renderer can forget information: endpoint order forgets braid winding,
squaring forgets sign, rounding hides tiny numeric changes. The action must then
show a related readout/history/animation/selection or report an unsupported
visual interpretation. A random blink does not repair missing semantics.

For animation, the meaningful result may be a nonempty finite sequence whose
intermediate frames differ even when the final pose agrees. This first contract
certifies a resulting static reference frame. The full scene must include its
signed history/animation state or supply a separately checked trace contract;
no endpoint-only equality shortcut is authorized.

## 4. A redraw is a debt, not an optional return value

The unrestricted classifier is for mathematical/reference tests. The application
entry `owe_released_response` returns `L1 IO (ResponseDebt ...)`. `ResponseDebt`
is opaque outside `Delivery.idric`. Its consumers cannot destructure the
classification and silently discard the redraw requirement.

`deliver_response` consumes the obligation and live window. It can return a
fixed-point proof and unchanged window, an exact-result submission receipt, a
native failure together with the still-owned debt, or a view-contract failure
with both debt and window retained. No exported debt-discard operation exists.
The returned resources themselves are linear, including failure paths.

Keep one obligation per accepted action. Rapid actions must not overwrite one
another, nor may two swaps be coalesced into an unchanged endpoint frame with
both marked delivered. Queue acknowledgement can immediately show the accepted
signed history while geometry animates; the acknowledgement must be related to
those pending operations. Batching requires an explicit per-event coverage
proof and is not supplied here.

The event driver must issue each release once. A resource API does not make
untrusted replayed Android events unique. Fresh event IDs, queue limits, visible
rejection of overflow and dispatcher integration are acceptance requirements,
not falsely claimed native implementations.

## 5. Direct NDK graphics protocol

`NativeBoundary` defines opaque `Window`, `Rasterized`, `Verified`, and
`Submitted` resource families. The type indices carry the same canvas, surface
epoch, viewport revision, frame revision, input serial and exact picture.
`submit_changed_result` invokes rasterize -> verify -> submit for the state
produced by the operation. A resource cannot be silently copied or dropped in
this linear API. Wrong epochs, frames, and pictures are negative compiler cases.

The native implementer must bind:

| Contract | Actual boundary and required evidence |
| --- | --- |
| `acquire_window` | live `ANativeWindow` reference, positive dimensions, EGL config/context/surface, owning render thread and fresh epoch |
| `rasterize` | current EGL context, correct framebuffer/viewport, full repaint or valid retained content, exact opaque RGB8 lowering and GLES error handling |
| `verify_pixels` | actual readback of the just-drawn framebuffer against expected samples; never fill observations from expected values or stale CPU cache |
| `submit` | check `eglSwapBuffers`, associate the exact frame/event IDs, preserve ordering; failure is not a receipt |
| `reuse_window` / `close_window` | consume ownership, release exactly once, handle context/window loss and recreation without stale handles |
| input acknowledgement | call `AInputQueue_finishEvent` exactly once promptly; do not confuse input processing with drawing or submission |

`Verified` is a trusted-driver assertion to be earned by checked readback, not a
pure proof that GPU hardware executed. `Submitted` means the buffer was posted,
not that the physical panel scanned it out. These foreign facts cannot be
manufactured by the dependent core. The first device acceptance must retain
actual readbacks/screenshots/trace IDs and a human-visible result.

No concrete NativeBoundary value is implemented in this branch. Its linear-IO
orchestration is type-level/executable host source, not an NDK ABI implementation,
a native Idriç build, a GPU test, or an APK. Implementing those fields by calling
Sokol or by copying the semantics into C would violate this branch's purpose.

## 6. Progress and failure

Types enforce valid relationships and ownership, not a real-time scheduler.
A stopped app, lost surface, hung driver, or infinite external wait cannot be
made to show a frame by naming a return type. The concrete driver needs a
bounded, monotonic-clock deadline, retained pending work, explicit failure,
and lifecycle recovery. No latency bound or physical-delivery theorem is
claimed before the dispatcher/backend is implemented and tested.

A minimal useful end-to-end acceptance trace is:

    native event serial and pointer
    -> normalized coordinate + current surface/viewport/frame key
    -> captured painted object identity
    -> revalidated release
    -> exact operation/current-state transition
    -> changed meaningful pixel/patch
    -> matching native readback
    -> checked buffer submission
    -> observed on-screen response

## Primary boundary references

- Android NDK input: https://developer.android.com/ndk/reference/group/input
- Android NDK native windows: https://developer.android.com/ndk/reference/group/a-native-window
- Android buffer queues and EGL: https://source.android.com/docs/core/graphics/arch-egl-opengl
- Khronos `eglSwapBuffers`: https://registry.khronos.org/EGL/sdk/docs/man/html/eglSwapBuffers.xhtml

These identify actual APIs and their limits; they are not evidence that this
branch has executed them.
