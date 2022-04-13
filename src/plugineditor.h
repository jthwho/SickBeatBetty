#pragma once

#include "pluginprocessor.h"
#include "beatgenclockui.h"

class PluginEditor  : public juce::AudioProcessorEditor {
    public:
        explicit PluginEditor(PluginProcessor &proc);
        ~PluginEditor() override;

        void paint(juce::Graphics &) override;
        void resized() override;

    private:
        PluginProcessor                     &_proc;
        BeatGenClockUI                      _clockUI;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
