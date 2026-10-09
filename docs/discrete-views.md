# Small views: braid B₂, a 3×3 board and binary tensors

These are separate experiments in one Swap app, not a claim that a tic-tac-toe
game is a braid or a Venn diagram. The three-strand triangle stays the default.

## Two strands

A single adjacent signed crossing generates B₂, which is isomorphic to Z.
The endpoint projection B₂ -> S₂ retains only parity. Thus crossing twice
with the same sign restores the endpoint order while retaining winding 2.
The 2-strand view uses its own history and shares Swap's existing crossing
model, duration, replay and explicit over/under input. It does not counterfeit
the 3-strand triangle.

## Three-by-three board

The 3×3 tic-tac-toe grid is an immediately drawable discrete surface with
two player marks and three row/column positions. Its small state machine
supports occupied-cell rejection, alternating turns, wins, draws and reset.
This is groundwork for other finite-grid actions, not a forced interpretation
of braids.

## Membership atoms and binary tensor tables

Two sets have four possible membership atoms: 00, 10, 01, 11. Their 2×2
table is a view of the tensor product of two two-element axes. Three sets
have eight atoms (a 2×2×2 table, visualized as cube vertices). For n sets,
a membership atom is an n-bit word and the table shape is 2×...×2, with
2^n entries. An intersection of all sets is just the all-ones atom;
one specific atom also fixes which sets are NOT included.

The model indexes atoms by a uint64 bit word (2..63 axes), with no
exponential-sized buffer. A visible 2×2 slice varies the first two axes while
holding remaining membership bits fixed. The UI initially offers 2..7 axes;
this is a drawing limit, not a theorem that Venn diagrams end at seven.

For two and three axes we can show the familiar overlapping circles and,
respectively, a 2×2 table or cube. For more axes, the tensor slice is an honest
stand-in rather than a false planar Venn drawing. No Bayesian model, p-adic
action, inference semantics, probabilities or arbitrary Venn topology has yet
been chosen. These are reusable binary indices for those future experiments.

Acceptance separates host state/geometry tests, ICK compile, Android native
link, graphical rendering and touch on a physical handset.
