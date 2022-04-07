#include "PluginProcessor.h"
#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &proc, juce::AudioProcessorValueTreeState &params) : 
    AudioProcessorEditor(&proc), 
    _proc(proc),
    _params(params)
{
    _labelLevel.setText("Level", juce::dontSendNotification);
    addAndMakeVisible(&_labelLevel); 
    addAndMakeVisible(&_sliderLevel);
    _attachLevel.reset(new SliderAttachment(_params, "level", _sliderLevel));

    setSize(400, 400);
}

PluginEditor::~PluginEditor() {

}

void PluginEditor::paint(juce::Graphics &g) {
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    return;
}

void PluginEditor::resized() {
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto r = getLocalBounds();
    auto gainRect = r.removeFromTop(40);
    _labelLevel.setBounds(gainRect.removeFromLeft(80));
    _sliderLevel.setBounds(gainRect);
    return;
}
