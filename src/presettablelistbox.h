#ifndef _PRESETTABLELISTBOX_H_
#define _PRESETTABLELISTBOX_H_
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "programmanager.h"

class PresetTableListBox :
    public juce::Component,
    public juce::TableListBoxModel 
{
    public:
        PresetTableListBox();
        ~PresetTableListBox();
        
        ProgramManager::PresetInfo PresetTableListBox::getSelectedInfo() const;
        
        int getNumRows();
        void paintRowBackground(juce::Graphics &g, int rowNumber, int width, int height, bool rowIsSelected);
        void paintCell(juce::Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected);
        void resized();

        void updatePresetList();

    private:
        juce::TableListBox                  _table;
        ProgramManager::PresetInfoArray     _presets;
        juce::Font                          _font { 14.0f };
};

#endif /* _PRESETTABLELISTBOX_H_ not defined */
