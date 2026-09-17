# Changelog

The original Debian-format history is in `changelog`.

## 1.0.0 (2026-09-17)

The revival release. Same synth, new build, new formats.

### Added
- CLAP plugin, through clap-juce-extensions.
- A headless test that renders a note, checks determinism, and loads every factory patch.
- CI on Linux, macOS and Windows, with clap-validator, lilv and auval; release packages for
  Linux x86_64 and arm64, macOS universal, Windows x64 and arm64.
- The factory bank is found in a `patches` folder next to the binary, so a download works
  without an installer.

### Changed
- CMake on JUCE 8.0.15 replaces the bundled JUCE 5.3.2, the VST3 SDK, the Projucer output
  and the Xcode, Visual Studio and Makefile projects.
- A parameter set by the host is applied to the engine's control at once, so a host that sets
  and reads back without processing sees its value (this is what a CLAP flush does).
- The Windows build uses MSVC with the C++ runtime linked statically.

### Removed
- VST2 and AAX: their SDKs are no longer distributable.

### Fixed
- The filter style was not refreshed when the window first opened (mtytel/helm#275, xard-dev).
- An unsigned negation in the fixed-point oscillator (jjYBdx4IL/helm-portable).
