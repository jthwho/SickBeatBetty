#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

PluginEditor::PluginEditor(PluginProcessor &proc) : 
    AudioProcessorEditor(&proc), 
    _proc(proc),
    _beatGenTabs(juce::TabbedButtonBar::TabsAtTop),
    _tooltipWindow(this)
{
    setTitle("Sick Beat Betty");
    setResizable(true, false);
    for(int i = 0; i < PluginProcessor::beatGenCount; i++) {
        _beatGenUI.add(std::make_unique<BeatGenUI>(_proc.beatGen(i)));
        _beatGenTabs.addTab(juce::String::formatted("Gen %d", i + 1), juce::Colour(32, 32, 32), _beatGenUI[i], false);
    }
    addAndMakeVisible(_beatGenTabs);
    addAndMakeVisible(_tooltipWindow);
    setSize(960, 540);
    setResizeLimits(960, 540, 9999, 9999);
    // FIXME: Setting up an icon seems to segfault.  Need to figure this out later.
    //getPeer()->setIcon(juce::ImageFileFormat::loadFrom(BinaryData::drum_png, BinaryData::drum_pngSize));
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
