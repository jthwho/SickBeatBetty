#pragma once

#include "pluginprocessor.h"
#include "beatgenui.h"
#include "paramslider.h"
#include "aboutui.h"
#include "programeditor.h"

class PluginEditor  : 
    public juce::AudioProcessorEditor,
    public juce::MenuBarModel 
{
    public:
        explicit PluginEditor(PluginProcessor &proc, juce::AudioProcessorValueTreeState &params);
        ~PluginEditor() override;

        void paint(juce::Graphics &) override;
        void resized() override;

        void loadPreset();
        void savePreset();

    protected:
        juce::StringArray getMenuBarNames();
        juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String &menuName);
        void menuItemSelected (int menuItemID, int topLevelMenuIndex);

    private:
        PluginProcessor                     &_proc;
        juce::MenuBarComponent              _menuBar;
        std::unique_ptr<ParamSlider>        _bpm;
        std::unique_ptr<juce::Label>        _bpmLabel;
        juce::TabbedComponent               _beatGenTabs;
        juce::OwnedArray<BeatGenUI>         _beatGenUI;
        juce::TooltipWindow                 _tooltipWindow;
        AboutUI                             _aboutUI;
        ProgramEditor                       _programEditor;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
