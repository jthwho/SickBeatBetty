#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "beatgengroup.h"
#include "applogger.h"

class PluginProcessor  : public juce::AudioProcessor {
    public:
        typedef std::unique_ptr<juce::XmlElement> StateXML;

        static const int beatGenCount = 8;

        PluginProcessor();
        ~PluginProcessor() override;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;

        bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

        void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
        using AudioProcessor::processBlock;

        juce::AudioProcessorEditor *createEditor() override;
        bool hasEditor() const override;

        const juce::String getName() const override;

        bool acceptsMidi() const override;
        bool producesMidi() const override;
        bool isMidiEffect() const override;
        double getTailLengthSeconds() const override;

        int getNumPrograms() override;
        int getCurrentProgram() override;
        void setCurrentProgram(int index) override;
        const juce::String getProgramName(int index) override;
        void changeProgramName(int index, const juce::String &newName) override;

        void getStateInformation(juce::MemoryBlock &destData) override;
        void setStateInformation (const void *data, int sizeInBytes) override;

        StateXML getStateXML();
        bool setStateXML(const StateXML &xml);

        BeatGen &beatGen(int index);
        const BeatGen &beatGen(int index) const;

        juce::ValueTree &propsValueTree();
        const juce::ValueTree &propsValueTree() const;

    private:
        // Order here maters.  There are init dependency on each other.
        int                                                     _index;
        BeatGenGroup                                            _beatGen;
        // The params tree holds values that are shared between us and the host.
        juce::AudioProcessorValueTreeState                      _params;
        // The props are properties that need to be stored, but are not shared with the host.
        juce::ValueTree                                         _props;
        bool                                                    _transportRunning = false;
        std::atomic<float>                                      *_bpm = nullptr;
        double                                                  _sampleRate = 0.0;
        double                                                  _now = 0.0;

        bool setStateXMLv1(const StateXML &xml);
         

        juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() const;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};

inline BeatGen &PluginProcessor::beatGen(int index) {
    return _beatGen[index];
}

inline const BeatGen &PluginProcessor::beatGen(int index) const {
    return _beatGen[index];
}

inline juce::ValueTree &PluginProcessor::propsValueTree() {
    return _props;
}

inline const juce::ValueTree &PluginProcessor::propsValueTree() const {
    return _props;
}
