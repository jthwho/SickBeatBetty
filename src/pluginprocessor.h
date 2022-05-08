#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "beatgengroup.h"
#include "applogger.h"
#include "programmanager.h"

class PluginProcessor  : 
    public juce::AudioProcessor,
    public ProgramManager::Listener
{
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

        BeatGen &beatGen(int index);
        const BeatGen &beatGen(int index) const;

        ProgramManager &programManager();
        const ProgramManager &programManager() const;

    private:
        // Order here maters.  There are init dependency on each other.
        int                                                     _index;
        BeatGenGroup                                            _beatGen;
        // The params tree holds values that are shared between us and the host.
        juce::AudioProcessorValueTreeState                      _params;
        bool                                                    _transportRunning = false;
        std::atomic<float>                                      *_bpm = nullptr;
        double                                                  _sampleRate = 0.0;
        double                                                  _now = 0.0;
        ProgramManager                                          _programManager;
         
        juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() const;
        void programManagerProgramChanged(int value) override;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};

inline BeatGen &PluginProcessor::beatGen(int index) {
    return _beatGen[index];
}

inline const BeatGen &PluginProcessor::beatGen(int index) const {
    return _beatGen[index];
}

inline ProgramManager &PluginProcessor::programManager() {
    return _programManager;
}

inline const ProgramManager &PluginProcessor::programManager() const {
    return _programManager;
}
