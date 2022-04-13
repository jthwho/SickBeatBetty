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

// Data class to link all the different ways a parameter might be accessed.
class ParamValue {
    public:
        typedef std::vector<ParamValue *> PtrList;
        typedef std::unique_ptr<juce::RangedAudioParameter> RangedAudioParamUniq;
        typedef std::function<RangedAudioParamUniq(const ParamValue &)> RangedAudioParamFunc;

        ParamValue() { 

        }

        bool attach(const juce::AudioProcessorValueTreeState &state) {
            _value = state.getRawParameterValue(_id);
            _param = state.getParameter(_id);
            return _value != nullptr;
        }

        void setup(PtrList &list, const juce::String &id, const juce::String &name, RangedAudioParamFunc func, int moduleID, int index = 0) {
            jassert(_id.isEmpty()); // Make sure setup isn't called twice.
            _id = id;
            _name = name;
            _func = func;
            _moduleID = moduleID;
            _index = index;
            list.push_back(this);
            return;
        }

        RangedAudioParamUniq layout() const {
            return _func(*this);
        }

        void notifyHost() const {
            // This is a really lame hack to ensure the new value doesn't
            // match the previous value as there's no way to force an
            // update if the new value matches the previous value.
            float newValue = *_value + 0.000001f;
            _param->sendValueChangedMessageToListeners(newValue);
            _param->beginChangeGesture();
            _param->setValueNotifyingHost(newValue);
            _param->endChangeGesture();
            return;
        }

        int moduleID() const { return _moduleID; }
        juce::String id() const { return _id; }
        juce::String name() const { return _name; }
        juce::RangedAudioParameter *param() const { return _param; }
        int index() const { return _index; }
        float value() const { return *_value; }
        bool valueBool() const { return *_value >= 0.5; }
        int valueInt() const { return (int)*_value; }

    private:
        juce::String                _id;
        juce::String                _name;
        RangedAudioParamFunc        _func = nullptr;
        int                         _moduleID = 0;
        int                         _index = 0;
        std::atomic<float>          *_value = nullptr;
        juce::RangedAudioParameter  *_param = nullptr;
};

class BeatGen : public juce::AudioProcessorValueTreeState::Listener {
    public:
        enum ParamID {
            ParamEnabled            = 1,
            ParamNote               = 2,
            ParamSteps              = 3,
            ParamPhaseOffset        = 4,
            ParamBars               = 5,
            ParamLevel              = 6,
            ParamClockEnabled       = 7,
            ParamClockRate          = 8,
            ParamClockPhaseOffset   = 9,
            ParamClockMixMode       = 10,
            ParamClockLevel         = 11
        };
        
        struct GenerateState {
            bool    enabled = true;
            double  start = 0.0;      // Start of generate window in bar units
            double  end = 0.0;        // End of generate window in bar units
            double  stepSize = 0.0;   // Step size in bar units.
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

        const ParamValue *getParameter(int id, int index = 0) const;

        std::unique_ptr<juce::AudioProcessorParameterGroup> createParameterLayout() const;
        
        // Must be called after we create a parameter layout.
        void attachParams(juce::AudioProcessorValueTreeState &params);
        void generate(const GenerateState &state, juce::MidiBuffer &midi);
        void parameterChanged(const juce::String &parameterID, float newValue);

    private:
        int                                     _index = 0;
        int                                     _lastNote = -1;
        std::vector<Beat>                       _beats;
        std::atomic<bool>                       _needsUpdate = false;
    
        // Parameters
        ParamValue::PtrList         _params;
        ParamValue                  _enabled;
        ParamValue                  _note;
        ParamValue                  _steps;
        ParamValue                  _phaseOffset;
        ParamValue                  _bars;
        ParamValue                  _level;
        ParamValue                  _clockEnabled[maxClockCount];
        ParamValue                  _clockRate[maxClockCount];
        ParamValue                  _clockPhaseOffset[maxClockCount];
        ParamValue                  _clockMixMode[maxClockCount];
        ParamValue                  _clockLevel[maxClockCount];
        
        double levelAtPhase(double phase) const;
        void updateBeats();

        // Helper functions to map the clockRate floating point value to the clock rate integer value.
        int clockRateFloatToInt(float val) const;
        float clockRateIntToFloat(int value) const;

        static int nextIndex(); // FIXME: This is lame.
};

inline int BeatGen::index() const {
    return _index;
}
