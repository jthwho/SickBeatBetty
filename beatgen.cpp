#include "beatgen.h"

static const char *noteName[] = {
    "C",        // 0
    "C#",       // 1
    "D",        // 2
    "D#",       // 3
    "E",        // 4
    "F",        // 5
    "F#",       // 6
    "G",        // 7
    "G#",       // 8
    "A",        // 9
    "A#",       // 10
    "B"         // 11
};

void midiNoteToOctaveNote(int midiNote, int &octave, int &note) {
    octave = (midiNote / 12) - 1;
    note = midiNote % 12;
    return;
}

static juce::String midiNoteToString(int midiNote, int maxLen) {
    char buf[32] = { 0 };
    int octave, note;
    midiNoteToOctaveNote(midiNote, octave, note);
    snprintf(buf, sizeof(buf) - 1, "%d - %s", midiNote, noteName[note]);
    return buf;
}

static int stringToMidiNote(const juce::String &str) {
    int ret = 0;
    // FIXME!
    return ret;
}

BeatGen::BeatGen(int index) :
    _index(index)
{

}

BeatGen::~BeatGen() {

}

void BeatGen::attachParameters(juce::AudioProcessor &ap) {
    auto group = std::make_unique<juce::AudioProcessorParameterGroup>(
        juce::String::formatted("beatgen%d", _index),
        juce::String::formatted("Beat Gen %d", _index + 1),
        juce::String("|")
    );

    
    auto bpm = std::make_unique<juce::AudioParameterFloat>(
        juce::String("bpm"),
        juce::String("BPM"),
        1.0f, 999.0f, 120.0f
    );
    bpm->addListener(&_bpm);
    group->addChild(std::move(bpm));

    auto note = std::make_unique<juce::AudioParameterInt>(
        juce::String("note"),
        juce::String("Note"),
        0, 127, (36 + _index) % 128, // min, max, default
        juce::String(),
        &midiNoteToString,
        &stringToMidiNote
    );
    note->addListener(&_note);
    group->addChild(std::move(note));

    auto masterClock = std::make_unique<juce::AudioParameterInt>(
        juce::String("master_clock"),
        juce::String("Total Clocks"),
        1, maxClockRate, 16,
        juce::String()
    );
    masterClock->addListener(&_masterClock);
    group->addChild(std::move(masterClock));

    auto bars = std::make_unique<juce::AudioParameterInt>(
        juce::String("bars"),
        juce::String("Total Bars"),
        1, maxBars, 1,
        juce::String()
    );
    bars->addListener(&_bars);
    group->addChild(std::move(bars));

    for(int i = 0; i < maxClockCount; i++) {
        auto cgroup = std::make_unique<juce::AudioProcessorParameterGroup>(
            juce::String::formatted("clock%d", i),
            juce::String::formatted("Clock %d", i + 1),
            juce::String("|")
        );
        auto enabled = std::make_unique<juce::AudioParameterBool>(
            juce::String("enabled"),
            juce::String("Enabled"),
            i == 0, // Only the first clock should be enabled by default
            juce::String()
        );
        enabled->addListener(&_clockEnabled[i]);
        cgroup->addChild(std::move(enabled));

        auto rate = std::make_unique<juce::AudioParameterInt>(
            juce::String("rate"),
            juce::String("Rate"),
            1, maxClockRate, 4,
            juce::String()
        );
        rate->addListener(&_clockRate[i]);
        cgroup->addChild(std::move(rate));

        group->addChild(std::move(cgroup));
    }
    
    ap.addParameterGroup(std::move(group));
    return;
}

