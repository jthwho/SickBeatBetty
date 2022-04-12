#pragma once

#include "PluginProcessor.h"

class PluginEditor  : public juce::AudioProcessorEditor {
    public:
        explicit PluginEditor(PluginProcessor &proc, juce::AudioProcessorValueTreeState &params);
        ~PluginEditor() override;

    
        void paint(juce::Graphics &) override;
        void resized() override;

    private:
        typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
        typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

        PluginProcessor                     &_proc;
        juce::AudioProcessorValueTreeState  &_params;
        juce::Label                         _labelLevel;
        juce::Slider                        _sliderLevel;
        std::unique_ptr<SliderAttachment>   _attachLevel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
