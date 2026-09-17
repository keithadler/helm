# Helm

**A free polyphonic synthesizer with lots of modulation, by Matt Tytel.** Two oscillators
with cross modulation and up to fifteen unison voices each, a sub oscillator with shuffle
waveshaping, blendable 12 and 24 dB filters, three LFOs, a step sequencer, a modulation
system with live visual feedback, and formant, stutter, delay, distortion and reverb effects.
32 voices. The interface is drawn with OpenGL.

This is a revival of the original, whose repository was archived in 2022. The same synth,
built on a current JUCE, in current plugin formats, on Linux, macOS and Windows, with tests
and CI. Matt Tytel's own successor to Helm is [Vital](https://vital.audio).

| | |
|---|---|
| Plugins | CLAP (`Helm.clap`), VST3 (`Helm.vst3`), Audio Unit (`Helm.component`, macOS), LV2 (`Helm.lv2`) |
| Standalone | `Helm.app` (macOS), `Helm` (Linux), `Helm.exe` (Windows): Helm's own application, with menus, computer-keyboard playing and audio settings |
| Platforms | Linux (x86_64 and arm64), macOS (Apple silicon and Intel), Windows (x64 and arm64) |
| License | GPLv3. Factory presets and fonts under their own licenses, in `patches/` and `fonts/` |

## Download

Builds for every platform are attached to each
[release](https://github.com/keithadler/helm/releases). Unzip, and either run the standalone
where it is, or copy a plugin into your plugin folder:

| Platform | CLAP | VST3 | LV2 | Audio Unit |
|---|---|---|---|---|
| Linux | `~/.clap/` | `~/.vst3/` | `~/.lv2/` | |
| macOS | `~/Library/Audio/Plug-Ins/CLAP/` | `~/Library/Audio/Plug-Ins/VST3/` | `~/Library/Audio/Plug-Ins/LV2/` | `~/Library/Audio/Plug-Ins/Components/` |
| Windows | `%COMMONPROGRAMFILES%\CLAP\` | `%COMMONPROGRAMFILES%\VST3\` | `%APPDATA%\LV2\` | |

**Presets.** The `patches` folder in the download is the factory bank. Helm looks for it next
to whatever you are running, so keep it beside the standalone or the plugin, or copy it to the
system location Helm has always used: `/usr/share/helm/patches` on Linux,
`/Library/Application Support/Audio/Presets/Helm` on macOS, `Public Documents\Helm\Patches` on
Windows. Your own patches go in your user folder, which Helm creates on first run.

The macOS build is unsigned. If macOS refuses to open it, remove the quarantine flag once:

```bash
xattr -dr com.apple.quarantine Helm.app
```

## Building

Requires CMake 3.22 and a C++17 compiler. JUCE and clap-juce-extensions are fetched by CMake.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build
```

Plugins land in `build/Helm_artefacts/Release/`, the standalone in
`build/HelmStandalone_artefacts/Release/`. A build in the source tree finds the repository's
`patches` folder on its own.

On Linux, install what JUCE needs first:

```bash
sudo apt-get install libasound2-dev libjack-jackd2-dev libx11-dev libxext-dev libxinerama-dev \
  libxcursor-dev libxrandr-dev libfreetype-dev libfontconfig1-dev libgl1-mesa-dev lv2-dev
```

On Windows, build with MSVC (the Visual Studio Build Tools) and `-G Ninja`.

The tests render audio without a window or an audio device: a note sounds, silence is silence,
the same note renders the same samples twice, and every factory patch loads and renders a
finite block. CI runs them on all three platforms, checks the LV2 bundle with lilv, the CLAP
with clap-validator, and the Audio Unit with auval.

## Features

 - 32 voice polyphony
 - Interactive visual interface
 - Powerful modulation system with live visual feedback
 - Dual oscillators with cross modulation and up to 15 unison oscillators each
 - Sub oscillator with shuffle waveshaping
 - Oscillator feedback and saturation for waveshaping
 - 12 different waveforms
 - Blending between 12 or 24dB low/band/high pass filter
 - Low/Band/High Shelf filters
 - 2 monophonic and 1 polyphonic LFO
 - Step sequencer
 - Lots of modulation sources including polyphonic aftertouch
 - Simple arpeggiator
 - Effects: Formant filter, stutter, delay, distortion, reverb

## What changed from the original

- CMake and JUCE 8 replace the bundled JUCE 5.3, Projucer output and IDE projects. Nothing in
  the synth changed; the source edits are the ones JUCE's API changes required.
- CLAP is new. VST2 and AAX are gone, since their SDKs can no longer be distributed.
- The factory bank is found next to the binary, so a download needs no installer.
- A parameter set by the host is visible to the host immediately, not after the next audio
  block; this is what made the CLAP validator pass.
- Two community fixes: the filter style not refreshing when the window first opens
  (mtytel/helm#275), and an unsigned negation in the oscillator.

## Credits

Helm was written by [Matt Tytel](https://tytel.org). The factory presets are by Matt Tytel and
Cris Owl Alvarez; the fonts are Roboto and Droid Sans Mono under the Apache License. The manual
is in `docs/helm_manual.pdf`. This revival is maintained at
[github.com/keithadler/helm](https://github.com/keithadler/helm); the original is at
[github.com/mtytel/helm](https://github.com/mtytel/helm).
