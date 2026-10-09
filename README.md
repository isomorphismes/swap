# Swap

A mathematical movement toy, beginning with three colored braid strands and an
animated, correspondingly colored equilateral triangle.

The application is called **Swap**; this repository currently remains
`isomorphismes/switch`.

The first development slice connects Android NDK packaging, a small native
renderer, and Idriç type/design sketches. Broader connections to Pascal rivers,
permutations, polygon/polyhedral symmetry, Seifert constructions and hyperbolic
geometry belong in research notes until deliberately selected for exploration.

No factorial drills, exhaustive enumeration, or general many-strand color
system is required for the starting scene.

## Rough.js reference and Field Mouse port

The early drawing research keeps an independent
[Rough.js browser demonstration](reference-code/roughjs/browser-demo.html)
and a [native Field Mouse translation of the Rough.js generator](reference-code/fieldmouse/README.md).
The translation includes seeded rough strokes, curves, polygons, rectangles,
circles/ellipses, SVG serialization, a launcher using the actual
`dilapidated-shed/fieldmouse` interpreter, and differential reference tests.

These are reference implementations, **not** a commitment to using Rough.js
or the Field Mouse port as Swap's production Android renderer.
