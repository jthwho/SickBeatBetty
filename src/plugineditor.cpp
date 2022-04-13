#include "PluginProcessor.h"
#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &proc) : 
    AudioProcessorEditor(&proc), 
    _proc(proc),
    _clockUI(proc.beatGen(0), 0)
{
    addAndMakeVisible(_clockUI);
    setSize(400, 400);
}

PluginEditor::~PluginEditor() {

}

void PluginEditor::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::azure);
    return;
}

void PluginEditor::resized() {
    // Mini-rant: The manual nature of the JUCE layout sucks.  Sure wish there was something
    // like QLayout from Qt for this.  
    // TODO: Reinvent QLayout for the JUCE world.
    auto r = getLocalBounds();
    _clockUI.setBounds(r);
    return;
}
