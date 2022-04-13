#pragma once

#include "pluginprocessor.h"
#include "beatgenui.h"

class PluginEditor  : public juce::AudioProcessorEditor {
    public:
        explicit PluginEditor(PluginProcessor &proc);
        ~PluginEditor() override;

        void paint(juce::Graphics &) override;
        void resized() override;

    private:
        PluginProcessor                     &_proc;
        juce::TabbedComponent               _beatGenTabs;
        juce::OwnedArray<BeatGenUI>         _beatGenUI;
        juce::TooltipWindow                 _tooltipWindow;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
