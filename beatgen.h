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

class BeatGen {
    public:
        static const int firstNote = 36;
        static const int maxClockCount = 4;
        static const int maxBars = 8;
        static const int maxClockRate = maxBars * 16;

        BeatGen(int index = 0);
        ~BeatGen();

        int index() const;

        void attachParameters(juce::AudioProcessorValueTreeState &params);
        void reset(float sampleRate);
        void reset();
        void processBlock(juce::AudioBuffer<float> &audio, juce::MidiBuffer &midi);

    private:
        int                 _index { 0 };
        bool                _started { false };
        float               _sampleRate { 48000.0f };
        int                 _currentTime { 0 };
        bool                _masterClockValue { false };
        bool                _clockValue[maxClockCount] { false };
        Latch               _clockLatch[maxClockCount];
        int                 _lastNote { 0 };
        double              _lastNoteOnTime { 0.0 };

        // Parameters
        std::atomic<float>  *_enabled = nullptr;
        std::atomic<float>  *_bpm = nullptr;
        std::atomic<float>  *_note = nullptr;
        std::atomic<float>  *_mclockRate = nullptr;
        std::atomic<float>  *_mclockPhaseOffset = nullptr;
        std::atomic<float>  *_bars = nullptr;
        std::atomic<float>  *_clockEnabled[maxClockCount] = { nullptr };
        std::atomic<float>  *_clockRate[maxClockCount] = { nullptr };
        std::atomic<float>  *_clockPhaseOffset[maxClockCount] = { nullptr };
};

inline int BeatGen::index() const {
    return _index;
}
