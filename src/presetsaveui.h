#ifndef _PRESETSAVEUI_H_
#define _PRESETSAVEUI_H_
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "pluginprocessor.h"
#include "valuetreetexteditor.h"

class PresetSaveUI :
    public juce::Component
{
    public:
        PresetSaveUI(PluginProcessor &p);
        ~PresetSaveUI();

        void paint(juce::Graphics &g) override;
        void resized() override;

    private:
        PluginProcessor         &_proc;
        juce::Label             _nameLabel;
        ValueTreeTextEditor     _name;
        juce::Label             _authorLabel;
        ValueTreeTextEditor     _author;
        juce::Label             _descLabel;
        ValueTreeTextEditor     _desc;
};

#endif
