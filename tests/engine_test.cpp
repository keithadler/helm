/* Helm - headless checks of the engine and the patch bank
 *
 * Copyright (C) 2026 Keith Adler. GPL-3.0-or-later.
 *
 * Runs without a window or an audio device: a SynthBase with no GUI renders a note, and every
 * factory patch loads and renders a block. Exit status is the verdict, as CTest expects.
 */
#include "JuceHeader.h"
#include "synth_base.h"
#include "load_save.h"
#include "helm_common.h"

#include <cmath>
#include <cstdio>
#include <vector>

namespace {

class HeadlessSynth : public SynthBase {
  public:
    HeadlessSynth() { }
    using SynthBase::processAudio;
  protected:
    const CriticalSection& getCriticalSection() override { return critical_section_; }
    SynthGuiInterface* getGuiInterface() override { return nullptr; }
  private:
    CriticalSection critical_section_;
};

int failures = 0;
void check(bool ok, const char* what) {
  std::printf("%s  %s\n", ok ? "ok  " : "FAIL", what);
  if (!ok) ++failures;
}

struct Rendered { double rms = 0.0; bool finite = true; std::vector<float> left; };

// The engine renders at most mopo::MAX_BUFFER_SIZE samples per call; the plugin and the standalone
// both chop host blocks to that, so the test does too.
Rendered render(HeadlessSynth& synth, int blocks, int block_size, bool play_note) {
  Rendered r;
  block_size = std::min(block_size, mopo::MAX_BUFFER_SIZE);
  synth.getEngine()->setSampleRate(44100);
  synth.getEngine()->setBufferSize(block_size);
  if (play_note)
    synth.getEngine()->noteOn(60.0, 0.8);
  AudioSampleBuffer buffer(2, block_size);
  double sum = 0.0; long count = 0;
  for (int b = 0; b < blocks; ++b) {
    buffer.clear();
    synth.processAudio(&buffer, 2, block_size, 0);
    const float* left = buffer.getReadPointer(0);
    for (int i = 0; i < block_size; ++i) {
      if (!std::isfinite(left[i])) r.finite = false;
      sum += double(left[i]) * left[i]; ++count;
      r.left.push_back(left[i]);
    }
  }
  r.rms = std::sqrt(sum / std::max<long>(1, count));
  return r;
}

}  // namespace

int main(int argc, char** argv) {
  ScopedJuceInitialiser_GUI juce_init;
  File patches = argc > 1 ? File(argv[1]) : File();

  // 1. a note sounds, and silence is silence
  {
    HeadlessSynth synth;
    Rendered silent = render(synth, 8, 256, false);
    check(silent.finite && silent.rms < 1e-6, "no note, no output");
    Rendered note = render(synth, 80, 256, true);
    check(note.finite, "note on: every sample finite");
    check(note.rms > 1e-3, "note on: audible output");
    std::printf("      rms %.4f over %zu samples\n", note.rms, note.left.size());
  }

  // 2. the same note renders the same samples twice
  {
    HeadlessSynth a, b;
    Rendered ra = render(a, 20, 256, true), rb = render(b, 20, 256, true);
    check(ra.left == rb.left, "rendering is deterministic");
  }

  // 3. every factory patch loads and renders a finite block
  if (patches.isDirectory()) {
    Array<File> files;
    patches.findChildFiles(files, File::findFiles, true, String("*.") + mopo::PATCH_EXTENSION);
    int loaded = 0, bad = 0;
    for (const File& file : files) {
      HeadlessSynth synth;
      std::map<std::string, String> gui_state;
      LoadSave::loadPatchFile(file, &synth, gui_state);
      Rendered r = render(synth, 24, 256, true);
      if (!r.finite || r.rms > 20.0) { ++bad; std::printf("      bad: %s (rms %.3f)\n", file.getFileName().toRawUTF8(), r.rms); }
      else ++loaded;
    }
    check(files.size() > 200, "factory bank found");
    check(bad == 0, "every factory patch renders finite, sane audio");
    std::printf("      %d patches rendered, %d bad\n", loaded, bad);
  }
  else
    std::printf("skip  no patch directory given\n");

  std::printf("%s: %d failure(s)\n", failures ? "FAILED" : "OK", failures);
  return failures ? 1 : 0;
}
