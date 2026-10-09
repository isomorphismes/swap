# Link a touch to an earlier picture and a response to a newer picture

A touch event's own serial is not the serial of the action that produced the
picture currently under the finger. `TouchEvent.event_serial` and
`FrameKey.input_serial` therefore have separate roles. The latter is a frame's
causative input; the former identifies this newly decoded input event.

The application-facing `owe_released_response` requires submitted-snapshot
facts for BOTH the press picture and the current release picture. Each fact is
indexed by the native boundary, frame key, and exact picture. The constructor
requires a native submission receipt; `InputFinished` cannot supply one.
The receipt is used as an erased fact here so its live linear resource can
still be consumed exactly once by `reuse_window`.

The function also requires an output key with proof that it uses the same
surface/viewport, has a newer frame revision, and names this release event as
its causative input. Its debt is tied to that same native boundary and output
key. Reusing the touched frame's key for the changed result is rejected.

These relationships prevent a reference picture that was never submitted from
silently becoming the application's touch source, or an old frame receipt from
being reused as proof of the response. They do not prove a snapshot is the
latest physical scanout: the native event/frame sequencer must validate actual
freshness, timestamps, output-key reservation, and duplicate events. A snapshot
is a reusable fact of submission, not a live window reference or a physical
display acknowledgement.
