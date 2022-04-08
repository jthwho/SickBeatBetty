#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class BeatGen {
    public:
        static const int firstNote = 36;
        static const int maxClockCount = 4;
        static const int maxBars = 8;
        static const int maxClockRate = maxBars * 16;

        BeatGen(int index = 0);
        ~BeatGen();

        int index() const;

        void attachParameters(juce::AudioProcessor &ap);

    private:
        template<typename TYPE>
        class ParamValue : public juce::AudioProcessorParameter::Listener {
            public:
                ParamValue(TYPE t = 0) : _value(t) { }
                TYPE get() const { return _value; }

            private:
                std::atomic<TYPE>   _value;

                void parameterValueChanged(int index, float val) { 
                    _value = val; 
                    return;
                }

                void parameterGestureChanged(int index, bool starting) {
                    return;
                }
        };

        int                 _index { 0 };
        ParamValue<float>   _bpm { 120.0 };
        ParamValue<int>     _note { firstNote };
        ParamValue<int>     _masterClock { 16 };
        ParamValue<int>     _bars { 1 };
        ParamValue<bool>    _clockEnabled[maxClockCount] { false };
        ParamValue<int>     _clockRate[maxClockCount] { 4 };

};

inline int BeatGen::index() const {
    return _index;
}
