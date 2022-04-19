#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

PluginEditor::PluginEditor(PluginProcessor &proc, juce::AudioProcessorValueTreeState &params) : 
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
    _beatGenTabs.addTab("About", juce::Colour(32, 32, 32), &_aboutUI, false);
    addAndMakeVisible(_beatGenTabs);
    addAndMakeVisible(_tooltipWindow);
    
    // FIXME: Setting up an icon seems to segfault.  Need to figure this out later.
    //getPeer()->setIcon(juce::ImageFileFormat::loadFrom(BinaryData::drum_png, BinaryData::drum_pngSize));

    auto bpmParam = params.getParameter("bpm");
    if(bpmParam != nullptr) {
        _bpm = std::make_unique<ParamSlider>(*bpmParam);
        _bpm->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 30);
        addAndMakeVisible(_bpm.get());
        _bpmLabel = std::make_unique<juce::Label>("BPMLabel", "BPM");
        addAndMakeVisible(_bpmLabel.get());
    }

    // These should be last as they trigger the resized()
    setSize(960, 540);
    setResizeLimits(960, 540, 9999, 9999);
}

PluginEditor::~PluginEditor() {

}

void PluginEditor::paint(juce::Graphics &g) {
    juce::ignoreUnused(g);
    return;
}

void PluginEditor::resized() {
    auto r = getLocalBounds();
    // Pretty hacky way to add the BPM slider, but it'll work.
    if(_bpm.get() != nullptr) {
        int w = 300;
        int h = 30;
        int x = r.getWidth() - w;
        int y = 0;
        _bpm->setBounds(x, y, w, h);
        _bpmLabel->setBounds(x - 40, y, 40, h);
        
    }
    _beatGenTabs.setBounds(r);
    return;
}
