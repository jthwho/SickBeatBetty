#include "beatgen.h"

typedef std::vector<bool>   BoolVector;

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

BoolVector generateEuclidBeat(int count, int total, int off = 0) {
        BoolVector ret;
        ret.assign(total, false);
        int bucket = 0;
        int offset = off + 1;
        for(int x = 0; x < total; x++) {
                bucket += count;
                if(bucket >= total) {
                        bucket -= total;
                        ret[(x + offset) % total] = true;
                }
        }
        return ret;
}

BoolVector mixBeats(const BoolVector &b1, const BoolVector &b2, int mode) {
        BoolVector ret;
        size_t total = b1.size();
        if(b2.size() != total) return ret;
        ret.assign(total, false);
        for(int x = 0; x < total; x++) {
                bool v1 = b1[x];
                bool v2 = b2[x];
                bool v = false;
                switch(mode) {
                        default:
                        case 0:  v = v1 && v2; break;
                        case 1:  v = v1 || v2; break;
                        case 2:  v = v1 != v2; break;
                        case 3:  v = !(v1 && v2); break;
                        case 4:  v = !(v1 || v2); break;
                        case 5:  v = !(v1 != v2); break;
                        case 6:  v = !v1 && v2; break;
                        case 7:  v = !v1 || v2; break;
                        case 8:  v = !v1 != v2; break;
                        case 9:  v = v1 && !v2; break;
                        case 10: v = v1 || !v2; break;
                        case 11: v = v1 != !v2; break;
                }
                ret[x] = v;
        }
        return ret;
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
    
    _level.setup(
        _params,
        juce::String::formatted(PARAM_PREFIX "%d_level", _index),
        juce::String::formatted("G%d Level", _index + 1),
        [](const ParamValue &p) {
                return std::make_unique<juce::AudioParameterFloat>(
                    p.id(), p.name(),
                    -1.0f, 1.0f, 1.0f
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
        juce::String::formatted("G%d Bars", _index + 1),
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
            juce::String::formatted("G%d Clock %d Euclid Enable", _index + 1, i + 1),
            [](const ParamValue &p) {
                return std::make_unique<juce::AudioParameterBool>(
                    p.id(), p.name(),
                    p.index() == 0, // Only the first clock should be enabled by default
                    juce::String()
                );
            },
            i
        );
        
        _clockLevel[i].setup(
            _params,
            juce::String::formatted(PARAM_PREFIX "%d_clock%d_level", _index, i),
            juce::String::formatted("G%d Clock %d Level", _index + 1, i),
            [](const ParamValue &p) {
                    return std::make_unique<juce::AudioParameterFloat>(
                        p.id(), p.name(),
                        -1.0f, 1.0f, 0.0f
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
            },
            i
        );

        _clockPhaseOffset[i].setup(
            _params,
            juce::String::formatted(PARAM_PREFIX "%d_clock%d_phase_offset", _index, i),
            juce::String::formatted("G%d Clock %d Phase Offset", _index + 1, i + 1),
            [](const ParamValue &p) {
                return std::make_unique<juce::AudioParameterFloat>(
                    p.id(), p.name(),
                    0.0f, 1.0f, 0.0f
                );
            },
            i
        );

        _clockMixMode[i].setup(
            _params,
            juce::String::formatted(PARAM_PREFIX "%d_clock%d_mix_mode", _index, i),
            juce::String::formatted("G%d Clock %d Mix Mode", _index + 1, i + 1),
            [](const ParamValue &p) {
                return std::make_unique<juce::AudioParameterInt>(
                    p.id(), p.name(),
                    0, 11, 0,
                    juce::String()
                );
            },
            i
        );

    }
}

BeatGen::~BeatGen() {

}

void BeatGen::parameterChanged(const juce::String &parameterID, float newValue) {
    juce::ignoreUnused(parameterID, newValue);
    _needsUpdate = true;
    return;
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
        params.addParameterListener(i->id(), this);
    }
    return;
}

void BeatGen::reset() {
    updateBeats();
    _lastNote = -1;
    return;
}

static double phaseMultiplyAndShift(double inputPhase, double multiply, double shift, double &phaseCount) {
    double ret = modf(inputPhase * multiply, &phaseCount);
    ret = modf(abs(ret + shift + 1.0), &phaseCount);
    return ret;
}

double BeatGen::levelAtPhase(double phase) const {
    double ret = _level.value();
    for(int i = 0; i < maxClockCount; i++) {
        double phaseCount;
        double clockLevel = phaseMultiplyAndShift(phase, _clockRate[i].value(), _clockPhaseOffset[i].value(), phaseCount);
        clockLevel *= _clockLevel[i].value();
        ret += clockLevel;
    }
    if(ret > 1.0) ret = 1.0;
    else if(ret < 0.0) ret = 0.0;
    return ret;
}

void BeatGen::updateBeats() {
    int steps = _mclockRate.valueInt();
    _beats.clear();
    BoolVector clock[maxClockCount];
    BoolVector beatClock;
    beatClock.assign(steps, true); // Start with all the beats turned on.
    for(int i = 0; i < maxClockCount; i++) {
        bool enabled = _clockEnabled[i].valueBool();
        if(enabled) {
            int rate = _clockRate[i].valueInt();
            int offset = (int)(_clockPhaseOffset[i].value() * (double)steps);
            int mode = _clockMixMode[i].valueInt();
            clock[i] = generateEuclidBeat(rate, steps, offset);
            beatClock = mixBeats(beatClock, clock[i], mode);
        }
    }
    for(int i = 0; i < steps; i++) {
        Beat beat;
        beat.start = (double)i / (double)steps;
        beat.velocity = beatClock[i] ? levelAtPhase(beat.start) : 0.0;
        _beats.push_back(beat);
    }
    return;
}

void BeatGen::generate(const GenerateState &state, juce::MidiBuffer &midi) {
    if(_needsUpdate) {
        _needsUpdate = false;
        updateBeats();
    }
    // Check all the beats and schedule the ones that occur during this generate period.
    int startPhase = (int)state.start;
    int endPhase = (int)state.end;
    bool enabled = state.enabled && _enabled.valueBool();
    int note = _note.valueInt();
    for(int phase = startPhase; phase <= endPhase; phase++) {
        for (auto &i : _beats) {
            double start = (double)phase + i.start;
            int offset = (int)round((start - state.start) / state.stepSize);
            if(start >= state.start && start < state.end) {
                if(_lastNote >= 0) {
                    midi.addEvent(juce::MidiMessage::noteOff(1, _lastNote), offset);
                    _lastNote = -1;
                }
                if(enabled && i.velocity > 0.0) {
                    //printf("G%d N%d %lf %lf %lf %lf %lf\n", _index, note, i.velocity, i.start, start, state.start, state.end);
                    midi.addEvent(juce::MidiMessage::noteOn(1, note, (float)i.velocity), offset);
                    _lastNote = note;
                }
            }
        }
    }
    return;
}


/*
void BeatGen::processBlock(double bpm, juce::AudioBuffer<float> &audio, juce::MidiBuffer &midi) {
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
    bool enabled = _enabled.value() >= 0.5;

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
        //if(masterClockEdge) printf("MCLK  %lf %lf %s %d\n", masterClockPhase, now, masterClockValue ? "RISE" : "FALL", i);

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
                if(enabled) {
                    int note = (int)_note.value();
                    _lastNote = note;
                    //printf("ON    %lf %lf %lf\n", phase, now, now - _lastNoteOnTime);
                    midi.addEvent(juce::MidiMessage::noteOn(1, note, (juce::uint8)110), i);
                    _lastNoteOnTime = now;
                }
            } else {
                //printf("OFF   %lf %lf\n", phase, now);
                midi.addEvent(juce::MidiMessage::noteOff(1, _lastNote), i);
            }
        }
    }
    return;
}
*/
