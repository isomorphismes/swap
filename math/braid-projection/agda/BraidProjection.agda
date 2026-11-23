{-# OPTIONS --safe --without-K #-}
module BraidProjection where

open import Agda.Builtin.Equality using (_≡_; refl)

-- No standard library, postulates, termination overrides or equality axioms.
data Empty : Set where

symmetry : {A : Set} {x y : A} → x ≡ y → y ≡ x
symmetry refl = refl

transitivity : {A : Set} {x y z : A} → x ≡ y → y ≡ z → x ≡ z
transitivity refl proof = proof

congruence : {A B : Set} (f : A → B) {x y : A} → x ≡ y → f x ≡ f y
congruence f refl = refl

data Crossing : Set where
  over-first under-first over-second under-second : Crossing

inverse : Crossing → Crossing
inverse over-first = under-first
inverse under-first = over-first
inverse over-second = under-second
inverse under-second = over-second

infixr 5 _∷_ _++_
data Word : Set where
  empty : Word
  _∷_ : Crossing → Word → Word

_++_ : Word → Word → Word
empty ++ right = right
(generator ∷ rest) ++ right = generator ∷ (rest ++ right)

-- A presentation as an inductively generated equivalence, not equality of words.
infix 4 _≈_
data _≈_ : Word → Word → Set where
  same : (word : Word) → word ≈ word
  reverse : {left right : Word} → left ≈ right → right ≈ left
  chain : {left middle right : Word} → left ≈ middle → middle ≈ right → left ≈ right
  prefix : {left right : Word} (generator : Crossing) →
    left ≈ right → (generator ∷ left) ≈ (generator ∷ right)
  cancel : (generator : Crossing) (rest : Word) →
    (generator ∷ inverse generator ∷ rest) ≈ rest
  artin : (rest : Word) →
    (over-first ∷ over-second ∷ over-first ∷ rest) ≈
    (over-second ∷ over-first ∷ over-second ∷ rest)

append-context : {left right : Word} → left ≈ right →
  (suffix : Word) → (left ++ suffix) ≈ (right ++ suffix)
append-context (same word) suffix = same (word ++ suffix)
append-context (reverse proof) suffix = reverse (append-context proof suffix)
append-context (chain first second) suffix =
  chain (append-context first suffix) (append-context second suffix)
append-context (prefix generator proof) suffix = prefix generator (append-context proof suffix)
append-context (cancel generator rest) suffix = cancel generator (rest ++ suffix)
append-context (artin rest) suffix = artin (rest ++ suffix)

record Action (State : Set) : Set where
  constructor action
  field
    move : Crossing → State → State
    cancellation : (generator : Crossing) (before : State) →
      move (inverse generator) (move generator before) ≡ before
    slide : (before : State) →
      move over-first (move over-second (move over-first before)) ≡
      move over-second (move over-first (move over-second before))
open Action

run : {State : Set} → Action State → Word → State → State
run model empty before = before
run model (generator ∷ rest) before = run model rest (move model generator before)

composition : {State : Set} (model : Action State) (left right : Word) (before : State) →
  run model (left ++ right) before ≡ run model right (run model left before)
composition model empty right before = refl
composition model (generator ∷ rest) right before =
  composition model rest right (move model generator before)

respects : {State : Set} (model : Action State) {left right : Word} →
  left ≈ right → (before : State) → run model left before ≡ run model right before
respects model (same word) before = refl
respects model (reverse proof) before = symmetry (respects model proof before)
respects model (chain first second) before =
  transitivity (respects model first before) (respects model second before)
respects model (prefix generator proof) before = respects model proof (move model generator before)
respects model (cancel generator rest) before =
  congruence (run model rest) (cancellation model generator before)
respects model (artin rest) before = congruence (run model rest) (slide model before)

-- Exactly the six valid endpoint orders. No repeated-identity alternative.
data Permutation : Set where
  order012 order021 order102 order120 order201 order210 : Permutation

first-swap : Permutation → Permutation
first-swap order012 = order102
first-swap order021 = order201
first-swap order102 = order012
first-swap order120 = order210
first-swap order201 = order021
first-swap order210 = order120

second-swap : Permutation → Permutation
second-swap order012 = order021
second-swap order021 = order012
second-swap order102 = order120
second-swap order120 = order102
second-swap order201 = order210
second-swap order210 = order201

endpoint-move : Crossing → Permutation → Permutation
endpoint-move over-first = first-swap
endpoint-move under-first = first-swap
endpoint-move over-second = second-swap
endpoint-move under-second = second-swap

first-involution : (before : Permutation) → first-swap (first-swap before) ≡ before
first-involution order012 = refl
first-involution order021 = refl
first-involution order102 = refl
first-involution order120 = refl
first-involution order201 = refl
first-involution order210 = refl

second-involution : (before : Permutation) → second-swap (second-swap before) ≡ before
second-involution order012 = refl
second-involution order021 = refl
second-involution order102 = refl
second-involution order120 = refl
second-involution order201 = refl
second-involution order210 = refl

endpoint-cancellation : (generator : Crossing) (before : Permutation) →
  endpoint-move (inverse generator) (endpoint-move generator before) ≡ before
endpoint-cancellation over-first before = first-involution before
endpoint-cancellation under-first before = first-involution before
endpoint-cancellation over-second before = second-involution before
endpoint-cancellation under-second before = second-involution before

endpoint-slide : (before : Permutation) →
  endpoint-move over-first (endpoint-move over-second (endpoint-move over-first before)) ≡
  endpoint-move over-second (endpoint-move over-first (endpoint-move over-second before))
endpoint-slide order012 = refl
endpoint-slide order021 = refl
endpoint-slide order102 = refl
endpoint-slide order120 = refl
endpoint-slide order201 = refl
endpoint-slide order210 = refl

endpoints : Action Permutation
endpoints = action endpoint-move endpoint-cancellation endpoint-slide

project : Word → Permutation
project word = run endpoints word order012

representative : Permutation → Word
representative order012 = empty
representative order021 = over-second ∷ empty
representative order102 = over-first ∷ empty
representative order120 = over-first ∷ over-second ∷ empty
representative order201 = over-second ∷ over-first ∷ empty
representative order210 = over-first ∷ over-second ∷ over-first ∷ empty

surjective : (order : Permutation) → project (representative order) ≡ order
surjective order012 = refl
surjective order021 = refl
surjective order102 = refl
surjective order120 = refl
surjective order201 = refl
surjective order210 = refl

-- A second action: signed exponent sum modulo 3. No integer axioms needed.
data Winding : Set where
  zero one two : Winding

positive : Winding → Winding
positive zero = one
positive one = two
positive two = zero

negative : Winding → Winding
negative zero = two
negative one = zero
negative two = one

winding-move : Crossing → Winding → Winding
winding-move over-first = positive
winding-move over-second = positive
winding-move under-first = negative
winding-move under-second = negative

winding-cancellation : (generator : Crossing) (before : Winding) →
  winding-move (inverse generator) (winding-move generator before) ≡ before
winding-cancellation over-first zero = refl
winding-cancellation over-first one = refl
winding-cancellation over-first two = refl
winding-cancellation under-first zero = refl
winding-cancellation under-first one = refl
winding-cancellation under-first two = refl
winding-cancellation over-second zero = refl
winding-cancellation over-second one = refl
winding-cancellation over-second two = refl
winding-cancellation under-second zero = refl
winding-cancellation under-second one = refl
winding-cancellation under-second two = refl

winding-slide : (before : Winding) →
  winding-move over-first (winding-move over-second (winding-move over-first before)) ≡
  winding-move over-second (winding-move over-first (winding-move over-second before))
winding-slide before = refl

winding-action : Action Winding
winding-action = action winding-move winding-cancellation winding-slide

winding : Word → Winding
winding word = run winding-action word zero

square-keeps-endpoints : project (over-first ∷ over-first ∷ empty) ≡ project empty
square-keeps-endpoints = refl

two-not-zero : two ≡ zero → Empty
two-not-zero ()

one-not-two : one ≡ two → Empty
one-not-two ()

square-not-identity : (over-first ∷ over-first ∷ empty) ≈ empty → Empty
square-not-identity proof = two-not-zero (respects winding-action proof zero)

over-not-under : (over-first ∷ empty) ≈ (under-first ∷ empty) → Empty
over-not-under proof = one-not-two (respects winding-action proof zero)

no-equality-reflection :
  ((left right : Word) → project left ≡ project right → left ≈ right) → Empty
no-equality-reflection reflect = square-not-identity
  (reflect (over-first ∷ over-first ∷ empty) empty refl)
