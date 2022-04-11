#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class Latch {
    public:
        Latch() :
            _output(),
            _latched()
        { }

        bool output() const {
            return _output;
        }

        bool latched() const {
            return _latched;
        }

        // Resets the latch
        void reset() {
                _output = false;
                _latched = false;
                return;
        }

        void input(bool val) {
            if(val) _latched = val;
            return;
        }

        void clock(bool val) {
            if(val) {
                // On a rising edge, the output will hold whatever value is in the latched and the latch value will be cleared.
                _output = _latched;
                _latched = false;
            } else {
                // On a falling edge, the output and
                _output = false;
            }
            return;
        }

    private:
        bool _output;
        bool _latched;
};

class ParamValue {
    friend class BeatGen; // FIXME: Move this into the generic version of what we finally inherit BeatGen from
    public:
        typedef std::vector<ParamValue *> PtrList;
        typedef std::unique_ptr<juce::RangedAudioParameter> RangedAudioParamUniq;
        typedef std::function<RangedAudioParamUniq(const ParamValue &)> RangedAudioParamFunc;

        ParamValue() { 

        }

        bool attach(const juce::AudioProcessorValueTreeState &state) {
            _value = state.getRawParameterValue(_id);
            return _value != nullptr;
        }

        RangedAudioParamUniq layout() const {
            return _func(*this);
        }

        juce::String id() const { return _id; }
        juce::String name() const { return _name; }
        int index() const { return _index; }
        float value() const { return *_value; }
        bool valueBool() const { return *_value >= 0.5; }
        int valueInt() const { return (int)*_value; }

    protected:
        void setup(PtrList &list, const juce::String &id, const juce::String &name, RangedAudioParamFunc func, int index = 0) {
            jassert(_id.isEmpty()); // Make sure setup isn't called twice.
            _id = id;
            _name = name;
            _func = func;
            _index = index;
            list.push_back(this);
            return;
        }

    private:
        juce::String            _id;
        juce::String            _name;
        RangedAudioParamFunc    _func = nullptr;
        int                     _index = 0;
        std::atomic<float>      *_value = nullptr;
};

class BeatGen : public juce::AudioProcessorValueTreeState::Listener {
    public:
        struct GenerateState {
            bool    enabled = true;
            double  start = 0.0;      // Start of generate window in phase units
            double  end = 0.0;        // End of generate window in phase units
            double  stepSize = 0.0;   // Step size in phase units.
        };

        struct Beat {
            double  start = 0.0;
            double  velocity = 0.0;
        };

        static const int firstNote = 36;
        static const int maxClockCount = 4;
        static const int maxBars = 8;
        static const int maxClockRate = maxBars * 16;

        BeatGen();
        ~BeatGen();

        int index() const;

        std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout() const;
        
        // Must be called after we create a parameter layout.
        void attachParams(juce::AudioProcessorValueTreeState &params);

        void reset(double sampleRate);
        void reset();
        void generate(const GenerateState &state, juce::MidiBuffer &midi);

        void parameterChanged(const juce::String &parameterID, float newValue);

    private:
        int                 _index = 0;
        int                 _lastNote = -1;
        std::vector<Beat>   _beats;
        std::atomic<bool>   _needsUpdate = false;

        // Parameters
        ParamValue::PtrList         _params;
        ParamValue                  _enabled;
        ParamValue                  _note;
        ParamValue                  _mclockRate;
        ParamValue                  _mclockPhaseOffset;
        ParamValue                  _bars;
        ParamValue                  _level;
        ParamValue                  _clockEnabled[maxClockCount];
        ParamValue                  _clockRate[maxClockCount];
        ParamValue                  _clockPhaseOffset[maxClockCount];
        ParamValue                  _clockMixMode[maxClockCount];
        ParamValue                  _clockLevel[maxClockCount];
        
        double levelAtPhase(double phase) const;
        void updateBeats();

        static int nextIndex(); // FIXME: This is lame.
};

inline int BeatGen::index() const {
    return _index;
}
