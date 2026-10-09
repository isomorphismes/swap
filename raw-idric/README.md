# Raw Idriç: touch must have its specified visible result

Active experiment on branch `raw-idric`. No Sokol, SDL, raylib, JavaScript
renderer, or C copy of application semantics is used by this package.
Inherited `src/` and `idric/` are earlier prototypes, not the raw implementation.
Their APK builds cannot qualify this package.

    pick the identity in the composed picture
    revalidate the captured pointer/identity on release
    apply its operation to the current state
    require a changed result pixel unless this state is already fixed
    retain a linear response obligation
    rasterize, verify, and submit that exact result through the NDK contract

## The condition

For operation f and state s, no semantic change is needed when f(s)=s. This is
not the same as saying f is globally idempotent. Reset may change its first
input but fix its own result.

For a changed state, `VisibleChange behaviour command before` requires a
bounded pixel in the operation's result region whose final opaque colour
differs between `draw before` and `draw (perform command before)`. A new revision,
callback name, object address, or control-only flash is not enough.

The reference classifier returns `AtFixedPoint`, `NeedsDrawing`, or
`UnobservableChange`. The last is a view-contract failure, not successful action
completion. For example, x=-1 -> x=1 is invisible in a display of x² alone;
showing the changed input or transformation can repair that interpretation.

## Source

- `Screen.idric`: bounded pixels, coupled paint/picking after composition,
  occlusion, and a hit proof for the exact painted pixel.
- `Contact.idric`: pointer/surface/viewport/frame validation, current-geometry
  identity revalidation, explicit cancellation, and release-to-current-state
  binding. Implemented activation is clicking, not the full braid drag policy.
- `Response.idric`: exact action/state/result-region/changed-pixel relationships.
- `Delivery.idric`: an opaque linear response debt. A failed native attempt
  returns the still-owned debt; a view failure retains both debt and window.
  Consumers cannot unwrap this obligation merely to drop the redraw.
- `Native.idric`: direct NDK/EGL/GLES implementer contract with linear resources
  and distinct rasterized, verified and submitted stages. Its orchestration
  function submits the exact result picture, not an arbitrary drawing.
- `Examples.idric`, `Tests.idric`, `ContactTests.idric`, `Run.idric`: exact small
  reference rasters, positive scenarios and compiler-rejected negative cases.

[Detailed contracts and remaining native obligations](contracts.md) describe
coordinate normalization, per-event delivery, rapid input, animation, foreign
returns, readback, buffer submission, lifecycle and progress.

## Qualification

CI boots `isomorphisms/Idric@94dfd99bd3e376507fedc8611053b7173b2519f0` and checks
the actual `.idric` files. It does not translate them to C, stock Idris or RefC.
The compiler checkout's linear-IO library is the explicit inherited effect
boundary. Indexed `data` declarations are necessary because its pinned
`choice ... one_of` grammar supports unindexed alternatives only.

No concrete NDK driver, native Idriç lowering, full Swap scene, GPU execution,
new APK or physical-screen result is supplied by these contracts. Their typecheck
and test result must be reported from the actual CI outcome, not inferred from
source presence.

A pixel difference does not prove human perceptibility; meaningful area,
contrast and duration remain acceptance requirements. The result-region
specification must itself be reviewed. Types cannot infer the intended picture.

`AInputQueue_finishEvent` acknowledges input processing, not display delivery.
`eglSwapBuffers` success is submission, not physical scanout. Linear types do
not force the OS to schedule a frame or establish a latency bound. Those facts
remain explicit rather than being fabricated as dependent proofs.
