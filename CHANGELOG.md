# Changelog

## 1.0.1 (2026-09-17)

### Fixed
- The Audio Unit crashed when a host restored state or changed program from a thread other
  than the message thread: the plugin updated its sliders directly on that thread. Found with
  pluginval at strictness 10 (a randomized suite crashed in "Plugin state" and "Background
  thread state"); after the fix, eight randomized suites pass. The full GUI refresh now runs on
  the message thread, at once when already there and otherwise on the next message pass.
- Restoring state (a preset load, a session reload) could be partly undone by parameter changes
  still queued for the audio thread from just before it. The plugin's state restore now goes
  through the same locked path as the standalone's, and that path drops the stale queue first.

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
