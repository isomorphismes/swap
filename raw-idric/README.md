# Raw Idriç: touch must have its specified aggregate visible result

Active experiment on branch `raw-idric`. This package uses no Sokol, SDL,
raylib, JavaScript renderer, or C copy of application semantics. Inherited
`src/` and `idric/` are earlier prototypes, not this implementation.

    pick the identity in the composed picture
    revalidate the captured pointer/identity on release
    apply its operation to the current state
    require aggregate change in the meaningful result region unless f(s)=s
    retain the linear response obligation
    rasterize, verify and submit that exact result through the NDK contract

## Aggregate means area AND amount of change

The earlier existential changed-pixel contract was wrong. It has been replaced,
not supplemented. `VisibleChange behaviour command before` now contains a proof
that the computed aggregate for the WHOLE declared result region passes the
operation's visibility policy. It contains no chosen changed-pixel witness.

The measurements are the region's actual address count, its changed-address
count, and the sum of absolute RGB channel differences between `draw before`
and `draw (perform command before)`. All addresses are enumerated once;
callers cannot supply duplicate samples, a selected changed subset, invented
counters, or another next picture. The policy requires minimum region size,
minimum changed area, fractional coverage AND mean colour difference.

[Source anchors, equations, budgets and limitations](aggregate-response.md)
record the Fourier-sound tests this restores. The 64-address/16-changed-address
budgets in the small reference fixtures are NOT calibrated phone thresholds.
The Fourier-derived >20% and mean RGB8 difference >1 are regression criteria,
not a theorem about perception or universal thresholds for every widget.

For f(s)=s, equality evidence permits no state redraw. Reset can still change
its first input. `UnobservableChange` means the visual contract failed, not that
the action succeeded. A counter or control-only flash outside the meaningful
result region cannot pay the response obligation. An x²-only view still cannot
explain x=-1 changing to x=1.

## Source

- `Screen.idric` couples paint and picking after clipping/composition/occlusion.
  Picking an individual address is appropriate for targeting, NOT for visual
  response certification.
- `Contact.idric` validates pointer/surface/viewport/frame and current identity.
- `Response.idric` computes whole-region metrics and checks the policy, indexed
  by exact operation/state/result drawing. No existential pixel escape hatch.
- `Guarantee.idric` requires the all-actions responsiveness law using that
  aggregate certificate. Native response debt therefore carries the stronger
  requirement without replacing its identity/frame/lifetime constraints.
- `Delivery.idric`, `Native.idric`, and `Snapshot.idric` retain exact-picture
  source/output receipts and linear rasterized/verified/submitted resources.
- `Examples.idric`, `Tests.idric`, `GuaranteeTests.idric`, `ContactTests.idric`
  retain positive/negative fixtures. The former one-pixel positive examples
  are now negative response fixtures (still useful for pointer-only tests).

## Qualification

CI uses `isomorphisms/Idric@94dfd99bd3e376507fedc8611053b7173b2519f0`, not a C
translation, stock Idris or RefC. Report its actual result. Source presence
is not successful typechecking. The aggregate revision has no locally available
Idriç compiler; its new positive proofs and failing cases await that CI run.

No concrete NDK driver, native Idriç lowering, full Swap scene, GPU execution,
new APK or physical-screen result is supplied. Device-sized meaningful regions,
perceptible aggregate budgets, duration and latency require device evidence.
Submission is not physical scanout. Types cannot force OS scheduling or prove
that a human noticed the result. These are unfinished acceptance obligations,
NOT permission to weaken aggregate response back to a pixel difference.

`aggregate-response.md` supersedes the changed-pixel descriptions in the earlier
`contracts.md` and `responsiveness-law.md`; their other identity/lifetime/native
requirements remain. Full animation traces and braid dragging remain separate.
