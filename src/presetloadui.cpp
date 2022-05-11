#include "presetloadui.h"

PresetLoadUI::PresetLoadUI(PluginProcessor &p) : 
    _proc(p),
    _loadButton("Load"),
    _cancelButton("Cancel")
{
    _loadButton.onClick = [this] {
        load();
    };
    _cancelButton.onClick = [this] {
        cancel();
    };
    addAndMakeVisible(_loadButton);
    addAndMakeVisible(_cancelButton);
    addAndMakeVisible(_presets);
    setSize(400, 300);
}

PresetLoadUI::~PresetLoadUI() {

}

void PresetLoadUI::paint(juce::Graphics &g) {
    juce::ignoreUnused(g);
    return;
}

void PresetLoadUI::resized() {
    auto r = getLocalBounds();

    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    using Px = juce::Grid::Px;
    using Item = juce::GridItem;
/*
    grid.templateRows = { Track(Fr(1)), Track(Fr(1)) };
    grid.templateColumns = { Track(Px(60)), Track(Fr(1)) };
    grid.items = { 
        Item(_nameLabel), Item(_name),
        Item(_authorLabel), Item(_author)
    };
    grid.performLayout(r.removeFromTop(60));
*/
    grid.templateRows = { Track(Fr(1)) };
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)) };
    grid.items = { Item(_loadButton), Item(_cancelButton) };
    grid.performLayout(r.removeFromBottom(30));

    _presets.setBounds(r);
    return;
}

void PresetLoadUI::load() {
    ProgramManager::PresetInfo info = _presets.getSelectedInfo();
    if(!info.isValid()) {
        juce::NativeMessageBox::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "Failed To Load Preset", "You should probably select a preset first.",
            this
        );
        return;
    }
    juce::XmlDocument doc(info.path);
    auto root = doc.getDocumentElement();
    if(root == nullptr) {
        juce::NativeMessageBox::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "Failed To Load Preset", "Failed to parse preset\n" + info.path.getFullPathName(),
            this
        );
        return;
    }
    if(!_proc.programManager().setStateFromXML(root)) {
        juce::NativeMessageBox::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "Failed To Load Preset", "Failed to read preset\n" + info.path.getFullPathName(),
            this
        );
        return;
    }
    closeDialog(0);
    return;
}

void PresetLoadUI::cancel() {
    closeDialog(1);
    return;
}

void PresetLoadUI::closeDialog(int ret) {
    juce::DialogWindow* dw = findParentComponentOfClass<juce::DialogWindow>();
    if(dw != nullptr) dw->exitModalState(ret);
    return;
}
