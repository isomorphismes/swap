# Idriç type sketches — not an implemented interface

This is mathematical/interface design. The notation below is deliberately
spatial and uses Unicode; it is not a promise that today's Idriç parser accepts
these declarations. No Idriç compiler or proof checker has run on this file.
The current executable is the small Icky C/NDK slice. A future typechecked core
must replace that semantic boundary explicitly, not claim this file already does.

## Identity is not position or appearance

```text
data Identity₃ : Type               data Slot₃ : Type
 identity : Fin 3 → Identity₃         slot : Fin 3 → Slot₃

                    inverse witnesses
Ordering₃ ≔ Slot₃  ⟷  Identity₃

colour : Identity₃ → RGB             -- three fixed values in the first scene
```

These are distinct wrappers, not two aliases for `Fin 3`: position and identity
remain different types even though they have the same number of elements. An ordering is a bijection, not an arbitrary list of three labels.
The renderer looks up colour by identity. Edge identity `i` means the triangle
edge opposite vertex identity `i`; this is the explicit correspondence used by
the current scene. Small marks 1/2/3 supplement, rather than replace, colour.

## Endpoints index each move and composition

```text
Adjacent₃ ≔ LeftPair | RightPair
Sense     ≔ Over | Under
exchange  : Adjacent₃ → Ordering₃ → Ordering₃

 p : Ordering₃      pair : Adjacent₃      sense : Sense
 ──────────────────────────────────────────────────────
 crossing pair sense : Step p (exchange pair p)

        ────────────────
        idle : Path p p

 first : Path p q       second : Path q r
 ────────────────────────────────────────
          first ▷ second : Path p r
```

The index enforces composable endpoints and preserved strand number. Signs
choose the crossing handedness; both signs have the same endpoint permutation.
The construction is not a split/merge network. A future river branch needs a
separate arity-changing and weight-conserving type, not a `Step p q` constructor.

## Recorded word, braid class, endpoint action are different types

```text
 recorded Path p q  ──quotient by braid equivalence──▶  Braid p q
          │                                             │
          └────────── endpoint permutation ─────────────┘
                                  │
                                  ▼
                       resting triangle action
```

Generate a congruence `≈ᵦ` from identity laws, inverse cancellation and the
three-strand relation (and allow it inside compositional contexts):

```text
 crossing pair Over ▷ crossing pair Under  ≈ᵦ  idle
 crossing pair Under ▷ crossing pair Over  ≈ᵦ  idle

          left ▷ right ▷ left   ≈ᵦ   right ▷ left ▷ right
```

Here the last line uses consistently positive crossings; its inverse follows
from the group laws. Do **not** add `left ▷ left ≈ᵦ idle`. That relation holds
only after passing to endpoint permutations. Nor does the stored word contain
every geometric detail: rates and particular continuous paths belong to a
realization, not to the combinatorial word alone.

An eventual proof obligation is

```text
                  history₁ ≈ᵦ history₂
 ────────────────────────────────────────────────────
 endpoint history₁ = endpoint history₂
```

The converse is false. The current C tests check endpoint compatibility of the
braid relation; they do not implement or validate a braid-equivalence solver.
A signed crossing sum is an invariant, not a complete equality algorithm.

## Triangle: exact resting action, numerical motion between rest states

All six endpoint permutations act as the full equilateral-triangle symmetry
group. The three even permutations are its in-plane rotations. The odd ones
are plane reflections at rest, realized visually by rigid half-turns in 3D.

```text
 p : Ordering₃                      step : Step p q
 ────────────────────               ─────────────────────────────────
 resting p : Pose₃                  turn step : Motion (resting p) (resting q)

                 t : UnitInterval
 ───────────────────────────────────────────────────────
 sample (turn step) t : ApproximateRigidTriangle Float32
```

Exact obligations: endpoint identity correspondence, opposite-edge colouring,
permutation composition, and the resting action respecting braid equivalence.
Numerical obligations: start/end sampling, preserved side lengths within a
stated error tolerance, no projection singularity, finite coordinates and a
nonzero separation between crossing strands in their 3D realization.

The half-turn path is a chosen visualization of a generator. Equivalent braid
words need not produce literally identical time-parameterized triangle movies.
The resting action factors through `S₃`; the movie is not a faithful model of
all braid history or ribbon framing.

## Keep the numerical/compiler boundary visible

```text
 exact mathematical plan           checked combinatorial transitions
            │                                   │
            └──────────────────┬────────────────┘
                               ▼
             numerical realization + tolerance obligations
                               │
                               ▼
                  machine coordinates / GPU triangles
```

Current realization: binary64 CPU geometry, conversion to binary32 drawing
coordinates, a fixed camera, three fixed colours. This is an explicit initial
choice, not a final Idriç scalar or surface-syntax decision. Shader execution
and numerical sampling cannot manufacture a proof of topological equivalence.

Later sketches may give a weighted binary-walk DAG its own interpretation,
or a finite-set action a geometric-symmetry witness. These are typed maps with
specific domains, not an untyped universal word that magically means every
mathematical system at once.

Reference: [Idris 2 dependent types tutorial](https://idris2.readthedocs.io/en/latest/tutorial/typesfuns.html)
for length-indexed vectors and finite-set building blocks; the particular
Swap interfaces and diagram syntax above are proposals, not quoted APIs.
