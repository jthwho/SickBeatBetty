#ifndef _PRESETLOADUI_H_
#define _PRESETLOADUI_H_
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "pluginprocessor.h"
#include "presettablelistbox.h"

class PresetLoadUI : 
    public juce::Component 
{
    public:
        PresetLoadUI(PluginProcessor &p);
        ~PresetLoadUI();

        void paint(juce::Graphics &g) override;
        void resized() override;

        void load();
        void cancel();

    private:
        PluginProcessor         &_proc;
        juce::TextButton        _loadButton;
        juce::TextButton        _cancelButton;
        PresetTableListBox      _presets;

        void closeDialog(int ret);
};

#endif /* _PRESETLOADUI_H_ not defined */
