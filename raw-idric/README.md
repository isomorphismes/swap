# Raw Idriç: touch must have its specified visible result

This is the active experiment on branch `raw-idric`. No Sokol, SDL, raylib,
JavaScript renderer, or C copy of the application semantics is used by this
package. Inherited `src/` and `idric/` are the earlier prototypes, not the raw
implementation. Their successful APK builds do not qualify this package.

The job is:

    pick the object in the picture that was drawn
    apply that object's operation to the correct state
    draw the resulting state
    require a changed result pixel unless the operation fixes this state
    rasterize, verify, and submit that exact picture through the NDK boundary

## The actual condition

For operation f and current state s, no semantic change is needed when f(s)=s.
This is a fixed-point condition, not the assertion that f is globally
idempotent. An idempotent reset may change its first input. A non-idempotent
operation may nevertheless have fixed points.

For a changed state, `VisibleChange behaviour command before` requires a bounded
pixel in the operation's declared result region whose final opaque colour differs
between `draw before` and `draw (perform command before)`. A changed revision,
allocated object, callback name, or control-only flash is not enough. The proof
is bound to the exact operation and before-state. It cannot certify an unrelated
next state or an old drawing.

`respond_to_action` computes an exact finite reference-raster check and returns
one of: `AtFixedPoint`, `NeedsDrawing`, or `UnobservableChange`. The last is a
contract failure and must not be reported as a successfully completed action.
It means the mathematical operation or view needs an explicit visible
interpretation. For example, x=-1 -> x=1 with a display of x² alone is invisible;
showing the changed input or its transformation repairs the view contract.

## What has code and what remains an obligation

- `Screen.idric`: bounded framebuffer addresses, a final picture carrying both
  pigment and target ownership, composition/occlusion, and a hit proof for the
  exact pixel. Picking does not keep an independent table of resting positions.
- `Response.idric`: state transition, result-region and changed-pixel witnesses,
  fixed-point decisions, and touch-to-action binding.
- `Native.idric`: direct NDK/EGL/GLES implementer contract with surface/frame/input
  indices, linear resources, distinct rasterized/verified/submitted stages, and
  an executable orchestration function against that contract. No concrete NDK
  implementation is supplied, so this is not native execution evidence.
- `Examples.idric` and `Tests.idric`: small exact reference pictures and positive
  and negative compilation cases. These are not the full braid/triangle/grid UI.

The compiler still has to check this exact source. CI boots the existing pinned
Idriç compiler; it does not translate the files into C, stock Idris or RefC.
The `linear` library is that compiler checkout's inherited linear-IO boundary,
not a replacement graphics engine. The few indexed `data` declarations are
needed because the pinned `choice ... one_of` grammar does not support indices.

## Limits that the types must not conceal

A reference pixel difference is not a proof that a human noticed it. Result-region
size, contrast, animation duration and full native-raster conformance require
acceptance tests. The result-region declaration itself is part of the reviewed
semantic specification; types cannot discover an author's intentions.

A real driver must validate finite coordinates, orientation, viewport, pointer
identity, lifecycle, render-thread ownership, actual readback and foreign return
codes. `AInputQueue_finishEvent` is not a display acknowledgement. Successful
`eglSwapBuffers` is submission to the native window, not proof of physical scanout.

Linear resources stop a checked caller from copying/dropping those resources.
They do not force Android to schedule the process or establish bounded latency.
No concrete driver, new APK, graphics test, physical-screen observation or
completed dependent proof is claimed merely by committing these files.

Full press/move/release capture, stale-frame revalidation, pending-action delivery,
rapid-input coverage and lifecycle retry contracts are specified in the next
boundary notes; they are not silently inherited from the earlier C adapter.
