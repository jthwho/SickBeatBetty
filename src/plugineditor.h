#pragma once

#include "pluginprocessor.h"
#include "beatgenui.h"
#include "paramslider.h"
#include "aboutui.h"

class PluginEditor  : public juce::AudioProcessorEditor {
    public:
        explicit PluginEditor(PluginProcessor &proc, juce::AudioProcessorValueTreeState &params);
        ~PluginEditor() override;

        void paint(juce::Graphics &) override;
        void resized() override;

    private:
        PluginProcessor                     &_proc;
        std::unique_ptr<ParamSlider>        _bpm;
        std::unique_ptr<juce::Label>        _bpmLabel;
        juce::TabbedComponent               _beatGenTabs;
        juce::OwnedArray<BeatGenUI>         _beatGenUI;
        juce::TooltipWindow                 _tooltipWindow;
        AboutUI                             _aboutUI;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
