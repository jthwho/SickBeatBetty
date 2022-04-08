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
    snprintf(buf, sizeof(buf) - 1, "%d - %s%d", midiNote, noteName[note], octave);
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

void BeatGen::attachParameters(juce::AudioProcessorValueTreeState &params) {
    
    params.createAndAddParameter(
        std::make_unique<juce::AudioParameterBool>(
            juce::String("enabled"),
            juce::String("Enabled"),
            false
        )
    );
    _enabled = params.getRawParameterValue("enabled");

    params.createAndAddParameter(
        std::make_unique<juce::AudioParameterFloat>(
            juce::String("bpm"),
            juce::String("BPM"),
            1.0f, 999.0f, 120.0f
        )
    );
    _bpm = params.getRawParameterValue("bpm");
    
    params.createAndAddParameter(
        std::make_unique<juce::AudioParameterInt>(
            juce::String("note"),
            juce::String("Note"),
            0, 127, (firstNote + _index) % 128,
            juce::String(),
            &midiNoteToString,
            &stringToMidiNote
        )
    );
    _note = params.getRawParameterValue("note");

    params.createAndAddParameter(
        std::make_unique<juce::AudioParameterInt>(
            juce::String("mclock_rate"),
            juce::String("Master Clock Rate"),
            1, maxClockRate, 16,
            juce::String()
        )
    );
    _mclockRate = params.getRawParameterValue("mclock_rate");

    params.createAndAddParameter(
        std::make_unique<juce::AudioParameterFloat>(
            juce::String("mclock_phase_offset"),
            juce::String("Master Clock Phase Offset"),
            -1.0f, 1.0f, 0.0f
        )
    );
    _mclockPhaseOffset = params.getRawParameterValue("mclock_phase_offset");

    params.createAndAddParameter(
        std::make_unique<juce::AudioParameterInt>(
            juce::String("bars"),
            juce::String("Total Bars"),
            1, maxBars, 1,
            juce::String()
        )
    );
    _bars = params.getRawParameterValue("bars");
    
    for(int i = 0; i < maxClockCount; i++) {
        juce::String id;
        int humanClockIndex = i + 1;
        id = juce::String::formatted("clock%d_enabled", i);
        params.createAndAddParameter(
            std::make_unique<juce::AudioParameterBool>(
                id,
                juce::String::formatted("Clock %d Enabled", humanClockIndex),
                i == 0, // Only the first clock should be enabled by default
                juce::String()
            )
        );
        _clockEnabled[i] = params.getRawParameterValue(id);

        id = juce::String::formatted("clock%d_rate", i);
        params.createAndAddParameter(
            std::make_unique<juce::AudioParameterInt>(
                id,
                juce::String::formatted("Clock %d Rate", humanClockIndex),
                1, maxClockRate, 4,
                juce::String()
            )
        );
        _clockRate[i] = params.getRawParameterValue(id);

        id = juce::String::formatted("clock%d_phase_offset", i);
        params.createAndAddParameter(
            std::make_unique<juce::AudioParameterFloat>(
                id,
                juce::String::formatted("Clock %d Phase Offset", humanClockIndex),
                -1.0f, 1.0f, 0.0f
            )
        );
        _clockPhaseOffset[i] = params.getRawParameterValue(id);
    }
    return;
}

void BeatGen::reset(double sampleRate) {
    _sampleRate = sampleRate;
    reset();
    return;
}

void BeatGen::reset() {
    printf("RESET %d %f\n", _currentTime, _sampleRate);
    _started = false;
    _currentTime = 0;
    _lastNoteOnTime = 0.0;
    _masterClockValue = false;
    for(int i = 0; i < maxClockCount; i++) {
        _clockValue[i] = false;
        _clockLatch[i].reset();
    }
    return;
}

static double phaseShiftAndMultiply(double inputPhase, double offset, double multiply, double &phaseCount) {
    double ret = modf(abs(inputPhase + offset + 1.0), &phaseCount);
    ret = modf(ret * multiply, &phaseCount);
    return ret;
}

void BeatGen::processBlock(juce::AudioBuffer<float> &audio, juce::MidiBuffer &midi) {
    if(*_enabled < 0.5f) {
        _started = false;
        return;
    }
    if(!_started) {
        printf("STARTED!\n");
        reset();
        _started = true;
    }

    //float *leftAudioData = audio.getWritePointer(0);
    //jassert(audio.getNumChannels() == 0); // We're a MIDI plugin, so we shouldn't have any audio.
    auto samples = audio.getNumSamples(); // But, we do get a sample count to we can keep track of time.

    double bpm = *_bpm;
    double bars = *_bars;
    double mclockRate = *_mclockRate;
    double mclockPhaseOffset = *_mclockPhaseOffset;
    double phaseLen = 60.0f / bpm * (4.0 * bars);  // Length of one phase in seconds.
    double clockPhaseOffset[maxClockRate];
    double clockRate[maxClockRate];

    for(int i = 0; i < maxClockCount; i++) {
        clockPhaseOffset[i] = *_clockPhaseOffset[i];
        clockRate[i] = *_clockRate[i];
    }

    // Walk forward in time and compute all the clocks.
    for(int i = 0; i < samples; i++, _currentTime++) {
        double now = (double)_currentTime / _sampleRate; // Current time in seconds.
        double phaseFull = now / phaseLen;
        double phaseCount;
        double phase = modf(phaseFull, &phaseCount); // phase will be a ramp 0.0 .. 0.999.. that will ramp over each phase

        double masterClockPhaseCount;
        double masterClockPhase = phaseShiftAndMultiply(phase, mclockPhaseOffset, mclockRate, masterClockPhaseCount);
        //leftAudioData[i] = masterClockPhase;

        bool masterClockValue = masterClockPhase < 0.5;
        bool masterClockEdge = masterClockValue != _masterClockValue;
        _masterClockValue = masterClockValue;
        if(masterClockEdge) printf("MCLK  %lf %lf %s %d\n", masterClockPhase, now, masterClockValue ? "RISE" : "FALL", i);

        double clockPhaseCount[maxClockCount];
        double clockPhase[maxClockCount];
        bool clockValue[maxClockCount];
        bool clockEdge[maxClockCount];

        for(int n = 0; n < maxClockCount; n++) {
            clockPhase[n] = phaseShiftAndMultiply(phase, clockPhaseOffset[n], clockRate[n], clockPhaseCount[n]);
            clockValue[n] = clockPhase[n] < 0.5;
            clockEdge[n] = clockValue[n] != _clockValue[n];
            _clockValue[n] = clockValue[n];
            //if(clockEdge[n]) printf("CLK %d %lf %lf %s\n", n, phase, now, _clockValue[n] ? "RISE" : "FALL");
        }

        bool latchClockEdge = masterClockEdge;
        bool latchClockValue = masterClockValue;
        for(int n = 0; n < maxClockCount; n++) {
            bool inEdge, inValue;
            if(*_clockEnabled[n] >= 0.5) {
                inEdge = clockEdge[n];
                inValue = clockValue[n];
            } else {
                inEdge = masterClockEdge;
                inValue = masterClockValue;
            }
            bool prevLatchOutput = _clockLatch[n].output(); // Store the current latch state so we'll know if it changed
            if(inEdge) _clockLatch[n].input(inValue);
            if(latchClockEdge) _clockLatch[n].clock(latchClockValue);
            latchClockValue = _clockLatch[n].output();
            latchClockEdge = prevLatchOutput != latchClockValue;
        }

        if(latchClockEdge) {
            if(latchClockValue) {
                int note = (int)*_note;
                _lastNote = note;
                printf("ON    %lf %lf %lf\n", phase, now, now - _lastNoteOnTime);
                midi.addEvent(juce::MidiMessage::noteOn(1, note, (juce::uint8)110), i);
                _lastNoteOnTime = now;
            } else {
                //printf("OFF   %lf %lf\n", phase, now);
                midi.addEvent(juce::MidiMessage::noteOff(1, _lastNote), i);
            }
        }
    }
    return;
}
