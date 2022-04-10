#include "beatgen.h"

#define PARAM_PREFIX    "beatgen"

static const char wholeNoteName[] = "CDEFGAB";
static const int wholeNoteOffset[] = {
    0,
    2,
    4,
    5,
    7,
    9,
    11
};
static const int wholeNoteCount = 7;

static const char *halfNoteName[] = {
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
static const int halfNoteCount = 12;

void midiNoteToOctaveNote(int midiNote, int &octave, int &note) {
    octave = (midiNote / halfNoteCount) - 2;
    note = midiNote % halfNoteCount;
    return;
}

static juce::String midiNoteToString(int midiNote, int maxLen) {
    juce::ignoreUnused(maxLen);
    char buf[32] = { 0 };
    int octave, note;
    midiNoteToOctaveNote(midiNote, octave, note);
    snprintf(buf, sizeof(buf) - 1, "%s%d", halfNoteName[note], octave);
    return buf;
}

static int stringToMidiNote(const juce::String &val) {
    auto str = val.trim().toUpperCase();
    // Check to see if we've been given a note.
    int note = -1;
    for(int i = 0; i < wholeNoteCount; i++) {
        if(str.startsWithChar(wholeNoteName[i])) {
            note = wholeNoteOffset[i];
            break;
        }
    }
    if(note != -1) {
        int octave = 4;
        bool neg = false;
        str = str.substring(1); // Remove the note
        if(str.startsWithChar('#')) {
            note++;
            str = str.substring(1); // Remove the sharp
        }
        if(str.startsWithChar('B')) {
            note--;
            str = str.substring(1); // Remove the flat
        }
        if(str.startsWithChar('-')) {
            neg = true;
            str = str.substring(1); // Remove the negative sign
        }
        octave = str.getIntValue(); // No way to know if this fails, which is dumb.
        if(neg) octave *= -1;
        octave += 2;
        octave *= halfNoteCount;
        note += octave;
    } else {
        note = str.getIntValue();
    }
    if(note < 0) note = 0;
    else if(note > 127) note = 127;
    return note;
}

int BeatGen::nextIndex() {
    static int index = 0;
    return index++;
}

BeatGen::BeatGen() :
    _index(nextIndex()) 
{
    _enabled.setup(
        _params, 
        juce::String::formatted(PARAM_PREFIX "%d_enabled", _index),
        juce::String::formatted("G%d Enabled", _index + 1), 
        [](const ParamValue &p) {
            return std::make_unique<juce::AudioParameterBool>(p.id(), p.name(), false);
        }
    );
    
    _note.setup(
        _params,
        juce::String::formatted(PARAM_PREFIX "%d_note", _index),
        juce::String::formatted("G%d Note", _index + 1),
        [this](const ParamValue &p) {
            return std::make_unique<juce::AudioParameterInt>(
                p.id(), p.name(),
                0, 127, (firstNote + _index) % 128,
                juce::String(),
                &midiNoteToString,
                &stringToMidiNote
            );
        }
    );
    
    _mclockRate.setup(
        _params,
        juce::String::formatted(PARAM_PREFIX "%d_mclock_rate", _index),
        juce::String::formatted("G%d Master Clock Rate", _index + 1),
        [](const ParamValue &p) {
            return std::make_unique<juce::AudioParameterInt>(
                p.id(), p.name(), 
                1, maxClockRate, 16,
                juce::String()
            );
        }
    );

    _mclockPhaseOffset.setup(
        _params,
        juce::String::formatted(PARAM_PREFIX "%d_mclock_phase_offset", _index),
        juce::String::formatted("G%d Master Clock Phase Offset", _index + 1),
        [](const ParamValue &p) {
                return std::make_unique<juce::AudioParameterFloat>(
                    p.id(), p.name(),
                    -1.0f, 1.0f, 0.0f
                );
        }
    );
    
    _bars.setup(
        _params,
        juce::String::formatted(PARAM_PREFIX "%d_bars", _index),
        juce::String::formatted(PARAM_PREFIX "G%d Bars", _index + 1),
        [](const ParamValue &p) {
            return std::make_unique<juce::AudioParameterInt>(
                p.id(), p.name(),
                1, maxBars, 1,
                juce::String()
            );
        }
    );
    
    for(int i = 0; i < maxClockCount; i++) {
        _clockEnabled[i].setup(
            _params,
            juce::String::formatted(PARAM_PREFIX "%d_clock%d_enabled", _index, i),
            juce::String::formatted("G%d Clock %d Enabled", _index + 1, i + 1),
            [this](const ParamValue &p) {
                return std::make_unique<juce::AudioParameterBool>(
                    p.id(), p.name(),
                    _index == 0, // Only the first clock should be enabled by default
                    juce::String()
                );
            }
        );
        
        _clockRate[i].setup(
            _params,
            juce::String::formatted(PARAM_PREFIX "%d_clock%d_rate", _index, i),
            juce::String::formatted("G%d Clock %d Rate", _index + 1, i + 1),
            [](const ParamValue &p) {
                return std::make_unique<juce::AudioParameterInt>(
                    p.id(), p.name(),
                    1, maxClockRate, 4,
                    juce::String()
                );
            }
        );

        _clockPhaseOffset[i].setup(
            _params,
            juce::String::formatted(PARAM_PREFIX "%d_clock%d_phase_offset", _index, i),
            juce::String::formatted("G%d Clock %d Phase Offset", _index + 1, i + 1),
            [](const ParamValue &p) {
                return std::make_unique<juce::AudioParameterFloat>(
                    p.id(), p.name(),
                    -1.0f, 1.0f, 0.0f
                );
            }
        );
    }
}

BeatGen::~BeatGen() {

}

std::unique_ptr<juce::AudioProcessorParameterGroup> BeatGen::createParameterLayout() const {
    auto group = std::make_unique<juce::AudioProcessorParameterGroup>(
        juce::String::formatted(PARAM_PREFIX "%d", _index),
        juce::String::formatted("Beat Gen %d", _index + 1),
        "|"
    );
    for(auto i : _params) {
        group->addChild(i->layout());
    }
    return group;
}

void BeatGen::attachParams(juce::AudioProcessorValueTreeState &params) {
    for(auto i : _params) {
        jassert(i->attach(params));
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

void BeatGen::processBlock(double bpm, juce::AudioBuffer<float> &audio, juce::MidiBuffer &midi) {
    if(_enabled.value() < 0.5f) {
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

    double bars = _bars.value();
    double mclockRate = _mclockRate.value();
    double mclockPhaseOffset = _mclockPhaseOffset.value();
    double phaseLen = 60.0f / bpm * (4.0 * bars);  // Length of one phase in seconds.
    double clockPhaseOffset[maxClockRate];
    double clockRate[maxClockRate];

    for(int i = 0; i < maxClockCount; i++) {
        clockPhaseOffset[i] = _clockPhaseOffset[i].value();
        clockRate[i] = _clockRate[i].value();
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
            if(_clockEnabled[n].value() >= 0.5) {
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
                int note = (int)_note.value();
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
