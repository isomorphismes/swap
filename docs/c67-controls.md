# Physical C67 input feedback

Observed on Android 14 MIRO C67: the first three-strand Swap APK starts,
and the animated triangle looks good. Buttons did not appear to work.
Some drags were not recognized; it was unclear how to choose over versus under.

The independent `idric` branch attempts a narrow control correction:

- Shared button rectangles for rendering and hit testing, moved up the screen.
- Press/release handling with cancellation and visible action notices.
- Input queued while an earlier 0.8-second triangle flip animates.
- DRAG OVER and DRAG UNDER explicit selections, applied to the grabbed colour.
- Horizontal adjacent dragging with threshold; release y does not change sign.
- C67 pixel coordinate cases and pure gesture cases in native and Idriç tests.
- Same triangle animation as the first accepted draft.

The new APK uses a separate package ID and does not replace the original.

Not yet confirmed on the phone: each of the nine buttons, both braid crossing
depths, frequent taps, all permissible adjacent drags, replay, pause, reset,
lifecycle cancellation and screen layout. A passing host test does not establish
these physical-touch results. Native Idriç Android lowering is a separate gap.
