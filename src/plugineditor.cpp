#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"
#include "presetloadui.h"
#include "presetsaveui.h"

#define MENU_NAME_PRESET    "Preset"

PluginEditor::PluginEditor(PluginProcessor &proc, juce::AudioProcessorValueTreeState &params) : 
    AudioProcessorEditor(&proc),
    MenuBarModel(),
    _proc(proc),
    _menuBar(this),
    _beatGenTabs(juce::TabbedButtonBar::TabsAtTop),
    _tooltipWindow(this),
    _programEditor(proc.programManager())
{
    addAndMakeVisible(_menuBar);
    addAndMakeVisible(_programEditor);

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
      _menuBar.setBounds(r.removeFromTop(25));
      _programEditor.setBounds(r.removeFromLeft(200));
      
    // Pretty hacky way to add the BPM slider, but it'll work.
    if(_bpm.get() != nullptr) {
        int w = 300;
        int h = 25;
        int x = r.getWidth() - w;
        int y = 0;
        _bpm->setBounds(x, y, w, h);
        _bpmLabel->setBounds(x - 40, y, 40, h);
        
    }
    _beatGenTabs.setBounds(r);
    return;
}

juce::StringArray PluginEditor::getMenuBarNames() {
    juce::StringArray ret = { 
        MENU_NAME_PRESET
    };
    return ret;
}

juce::PopupMenu PluginEditor::getMenuForIndex(int topLevelMenuIndex, const juce::String &menuName) {
    juce::ignoreUnused(topLevelMenuIndex);
    juce::PopupMenu ret;
    if(menuName == MENU_NAME_PRESET) {
        ret.addItem("Load Preset...", [this]{ loadPreset(); });
        ret.addItem("Save Preset...", [this]{ savePreset(); });
    }
    return ret;
}

void PluginEditor::menuItemSelected (int menuItemID, int topLevelMenuIndex) {
    juce::ignoreUnused(menuItemID, topLevelMenuIndex);
    return;
}

void PluginEditor::loadPreset() {
    juce::DialogWindow::LaunchOptions dl;
    dl.dialogTitle = "Load Preset";
    dl.componentToCentreAround = this;
    dl.useNativeTitleBar = false;
    dl.content.setOwned(new PresetLoadUI(_proc));
    dl.launchAsync();
    return;
}

void PluginEditor::savePreset() {
    juce::DialogWindow::LaunchOptions dl;
    dl.dialogTitle = "Save Preset";
    dl.componentToCentreAround = this;
    dl.useNativeTitleBar = false;
    dl.content.setOwned(new PresetSaveUI(_proc));
    dl.launchAsync();
    return;
}
