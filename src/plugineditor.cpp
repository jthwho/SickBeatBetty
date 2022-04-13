#include "PluginProcessor.h"
#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &proc) : 
    AudioProcessorEditor(&proc), 
    _proc(proc),
    _beatGenTabs(juce::TabbedButtonBar::TabsAtTop)
{
    setTitle("Sick Beat Betty");
    setResizable(true, false);
    for(int i = 0; i < PluginProcessor::beatGenCount; i++) {
        _beatGenUI.add(std::make_unique<BeatGenUI>(_proc.beatGen(i)));
        _beatGenTabs.addTab(juce::String::formatted("Gen %d", i + 1), juce::Colour(32, 32, 32), _beatGenUI[i], false);
    }
    addAndMakeVisible(_beatGenTabs);
    setSize(960, 540);
}

PluginEditor::~PluginEditor() {

}

void PluginEditor::paint(juce::Graphics &g) {
    juce::ignoreUnused(g);
    return;
}

void PluginEditor::resized() {
    auto r = getLocalBounds();
    _beatGenTabs.setBounds(r);
    return;
}
