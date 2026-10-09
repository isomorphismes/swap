# Mathematical connections to retain

The visual starting point is the user's river picture and movable physical
pieces, not a worksheet or a demand to enumerate combinations. The old Imgur
post has not been retrieved: these notes record the description in the design
conversation, not a claim to reproduce the original image.

Only three braid strands and a linked triangle belong to the current scene.
The following are possible investigations, not promised modes or a fixed plan.

## Braids, permutations and the triangle

Keep a signed record of crossings between adjacent positions. Composition
preserves order; opposite crossings cancel as braid classes. Repeating the
same signed crossing does not cancel, although its endpoint permutation does.
The projection is `B₃ → S₃`, not an identification of these two groups.

For an equilateral triangle, `S₃ ≅ D₃` (here `D₃` has six elements). Its cyclic
rotation subgroup `C₃` has three. A swap of two vertex positions fixes the third
vertex and exchanges the two corresponding opposite-side identities. Rotating
halfway around that fixed vertex's median realizes the reflection using a rigid
triangle in 3D. Two such swaps can return the triangle to its original resting
placement while retaining nontrivial braid history.

This exact all-permutations-as-symmetries correspondence is special to the
triangle. A general relabelling of a square's four vertices, for example, need
not be a square symmetry. Use an action that preserves the chosen geometric
structure. A faithful finite action embeds its group into a symmetric group;
Cayley's regular action embeds a finite group in the symmetric group on its
own underlying set. A single fixed small symmetric group does not contain all
finite groups.

## Rivers, Pascal addition and probability

Binary path histories can be grouped by their final displacement. In a merged
river picture, a channel's count is the sum of the counts entering it; that
local rule gives Pascal's triangle without starting from a factorial formula.
Equal independent binary choices give the binomial weights. Unequal choices
need probability weights, not just unweighted path counts.

Red/blue steps and a purple balance colour could accompany this later. Colour,
channel thickness and location represent distinct quantities and should not
be conflated. The central-limit interpretation needs centering and scaling,
as well as assumptions on the choices (independent identically distributed
finite-variance steps are a sufficient familiar case). An arbitrary branching
picture does not itself prove a central limit theorem.

A braid has a fixed number of strands: no splits or mergers. A river network
may change arity. They can share a visual scene or selected input data, but
must not be treated as the same mathematical object. In particular there is no
automatic canonical chain “binary tree → braid → Pascal statistic.” A claimed
interpretation needs its actual map and its preserved relations specified.

## Closures and Seifert-related directions

Closing a braid by joining corresponding bottom and top positions produces a
link. Following its components exposes the cycles of the endpoint permutation.
The permutation determines the number of components, not the full knot/link
type. A Seifert-surface construction could then supply a spanning sheet.

Ribbon framing adds twist data beyond an unframed braid. An object's final
pose need not encode that twist, which connects conceptually with the user's
Seifert ribbon work. Do not equate Seifert surfaces, Seifert-fibred spaces and
framed-ribbon motion merely because their names or drawings overlap. None of
these constructions or readout bindings is integrated in this first scene.

## Hyperbolic geometry and Indra's Pearls / Schottky

The quotient of `B₃` by its central full twist is the modular group
`PSL₂(ℤ)`, acting by Möbius transformations on the hyperbolic plane. That could
provide another view which forgets a specified part of braid history. It is
not the endpoint-permutation map and not the same as erasing all pure braids.

Holding two planar punctures fixed and moving a third around them gives the
fundamental group of the twice-punctured plane, a free group of rank two. A
chosen two-generator Schottky group provides a Möbius realization of that free
group. This is a particular connection, not an assertion that the entire
three-strand braid group is a Schottky group. Inverses matter: a binary tree
of positive words and the full reduced-word tree of a rank-two free group
are different objects.

These directions can reuse earlier hyperbolic visualization work after its
interfaces and exact mathematical maps are inspected. No new shared universal
geometry library, hyperbolic renderer or cross-repository API is invented here.

## Source anchors for the later mathematical work

- Joan Birman and Tara Brendle, *Braids: A Survey*: braid presentations,
  closures, permutation projection and related structures.
  https://arxiv.org/abs/math/0409205
- Daniel Glasscock, *What is a Braid Group?* (Ohio State-hosted exposition):
  geometric and algebraic starting points.
  https://math.osu.edu/sites/math.osu.edu/files/BraidGroup.pdf
- Charles Grinstead and J. Laurie Snell, *Introduction to Probability*:
  random walks, binomial probabilities and the central limit theorem.
  https://math.dartmouth.edu/~prob/prob/prob.pdf

The rigid-half-turn and opposite-edge correspondence in this prototype is
specified directly in the Idriç sketches and checked numerically in the model
tests. The source list is a starting bibliography, not a claim that any future
integration has already been proved or built.
