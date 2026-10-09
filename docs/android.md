# Android and compiler boundary

## Maintained route

The application owns mathematics, state, rendering, input and package identity.
Sokol supplies the native window/lifecycle and graphics wrapper. The pinned
`isomorphisms/android-NDK` substrate supplies its maintained direct
NativeActivity APK packager. Nothing from Swap's mathematics is copied into
that shared repository.

`DEPS.lock` records the exact Sokol, Android substrate, ICK and shared compiler
qualification revisions. Owned mathematical C expressions use `×` and `÷`;
foreign Sokol source and C pointer syntax retain their existing spellings.
Do not add a handwritten Unicode text substitution as a competing compiler.

The C frontend is ICK `c61e448251744a2f40ad743ebef1a027bdcd2f9d`, selected through
`isomorphisms/ai-ci` revision `015cc7901ae0b3ad262b476f24e129b53c56db95`.
The inspected reference is Seifert's division/readout branch
`aa829276287e98abfaaa28ae39598ec8a74c57db`, specifically its shared compiler
setup and `android/build-native.sh`. Its evidence is precedent, **not Swap
acceptance**. The shared action requalifies literal glyphs and the selected
Bionic boundary; the new app must still compile and run in its own right.

Stages are separate in `ci/build-toolchain.tsv`:

1. Pinned ICK compiles owned C and the included Sokol implementation to assembly.
2. Android NDK r27c (`27.2.12479018`) assembles and links for API 21, using the
   Android sysroot and runtime libraries. This platform assembly/link/runtime
   stage is the explicit ICK capability gap; no equivalent ICK platform stage
   has been qualified for Swap.
3. The shared Android packager can produce a candidate from that native library
   only when an explicitly enrolled signer is supplied. Publication remains a
   separate producer/version/update gate.

The host model and shared contract checker also use the pinned ICK compiler.
The workflow checks out the exact PR head, not an unlabelled synthetic merge.
It applies the shared `build-toolchain-v0` contract. No maintained stock GCC,
stock Clang-C, Gradle, Java, Kotlin or DEX alternate route is provided.

## Build

The current build entry deliberately requires Linux x86_64, the pinned NDK,
Git, Bash, and the qualified ABI-specific ICK compiler. Obtain the latter from
the shared `ai-ci/ick-android` action (or an independently verified local build
of exactly the same source/target). The workflow does this explicitly.

```sh
# Set ICK_CC and ANDROID_NDK_HOME to the qualified inputs for this ABI.
bash scripts/android-build.sh armeabi-v7a
# Use the separately qualified AArch64 compiler for:
bash scripts/android-build.sh arm64-v8a
```

Dependencies are fetched at immutable revisions. Existing dirty or differently
pinned checkouts are rejected, not reset. `libswap.so` is the native output;
its presence is not an installable APK or physical-device evidence. The link
rejects undefined symbols and checks `ANativeActivity_onCreate`.

Candidate package identity: `org.isomorphismes.swap`, launcher label `Swap`,
version code 1, minimum API 21, target API 35, no application DEX and no requested
permissions. OpenGL ES 3 is required by the selected Sokol backend. CPU targets
are ARM32 (`armeabi-v7a`, first MIRO A1 lane) and ARM64 (`arm64-v8a`). Compatibility
with an ABI is not proof of behavior on any particular handset.

## Signing and publication

No signer has been silently chosen for this new app. `android-package.sh`
requires explicit keystore path/type/alias/passwords and the expected
certificate SHA-256 digest. It never generates a key or substitutes a default.
A stable development signer must be deliberately enrolled for this package.
Only then run:

```sh
bash scripts/android-package.sh armeabi-v7a
```

The application verifies hashes of the native source, build recipe, dependency
pins, manifest and resulting library before packaging, and refuses a dirty
source checkout. The shared packager checks certificate, package, label, native payload and
absence of DEX. The resulting candidate must still pass the shared AICI
producer and version/update gate before distribution. This first PR adds no
store/publication workflow and no Android deployment permission.

## Acceptance boundaries and starting evidence

A local development probe executed 94,004 numerical checks on an ASCII-operator
projection of the same model/tests with GCC 14.2. This is mathematical/debug
feedback, **not** maintained ICK compiler, Sokol, Android, APK or device evidence.
The projection is not committed or offered as an alternate build route. The
maintained workflow tests the actual `×`/`÷` source with ICK.

Retained model cases cover all six resting orders, both pairs and signs,
constant triangle side lengths throughout the turns, a fixed rotation axis,
non-colliding crossing paths, colour/side identity correspondence, inverse vs
same-sign history, order dependence, endpoint compatibility of the braid
relation, pause/replay, invalid input and bounded history. Release/NDEBUG builds
keep these checks active. No complete braid equality solver is claimed.

GPU rendering and touch behavior still require native/device acceptance:
check all four crossings and opposite handedness; watch a triangle turn edge-on
and show its reverse side; confirm endpoint/side colours; compare two equal-sign
crossings with a move and its inverse; pause/replay; cancel a drag; background
and resume; inspect the 576×1152 layout. Process death currently resets the
in-memory scene; persistent sessions are outside this first slice.

References:
- https://github.com/isomorphisms/android-NDK/blob/42f04d7654d54309ec3f8787bfd2bc40730399d7/ARCHITECTURE.md
- https://github.com/isomorphisms/android-NDK/blob/42f04d7654d54309ec3f8787bfd2bc40730399d7/apk/README.md
- https://github.com/floooh/sokol/tree/401f21f8b7039258c35fef75c11d9a8a0e616771
- https://github.com/isomorphisms/ai-ci/tree/015cc7901ae0b3ad262b476f24e129b53c56db95/ick-android
