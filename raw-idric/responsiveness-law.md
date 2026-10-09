# The application must supply the law, not discover failure after each touch

The raw application entry requires `Responsive behaviour` as well as the
behaviour itself. Its field quantifies over EVERY action and EVERY state:

    show_changed_result :
      (command : action) → (before : model) →
      Not (perform behaviour command before = before) →
      VisibleChange behaviour command before

An application cannot enter the checked delivery path merely by defining a
state transition and an unrelated drawing function. It must supply a total
function explaining where each nontrivial result is visible. The witness is
indexed by that same behaviour, command and state. The examples supply proof
terms exhaustively for their small pair and formula state spaces, including
contradiction elimination for fixed inputs.

`respond_with_guarantee` uses this witness, not an exhaustive per-touch raster
search. `respond_to_action` remains a diagnostic that can identify an invisible
interpretation. `owe_released_response` uses only the certified function, so its
public path cannot construct the invisible-transition case. `deliver_response`
retains that defensive case for the shared reference-response representation.

This is a law about the declared reference picture and meaningful region. It
is not a proof that a driver executed, a compositor displayed the image, a
human noticed a one-pixel difference, or a deadline was met. The NDK and physical
acceptance obligations in `contracts.md` remain necessary. Native source/output
frame relationships are separately specified in `native-frame-link.md`.

The proof terms and negative cases have been written. Qualification requires
the pinned compiler to accept this exact commit and execute the fixtures.
