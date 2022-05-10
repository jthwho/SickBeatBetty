#include "presettablelistbox.h"

PresetTableListBox::PresetTableListBox() :
    juce::Component("PresetTableListBox"),
    juce::TableListBoxModel()
{
    addAndMakeVisible(_table);
    _table.getHeader().addColumn("Name", 1, 50, 50, -1);
    _table.getHeader().addColumn("Author", 2, 40, 40, -1);
    _table.getHeader().resizeAllColumnsToFit(true);
    _table.setModel(this);
    updatePresetList();
}

PresetTableListBox::~PresetTableListBox() {

}

int PresetTableListBox::getNumRows() {
    return _presets.size();
}

void PresetTableListBox::paintRowBackground(juce::Graphics &g, int rowNumber, int width, int height, bool rowIsSelected) {
    juce::Colour c = getLookAndFeel().findColour(juce::ListBox::backgroundColourId);
    if(rowIsSelected) c = c.brighter();
    g.setColour(c);
    g.fillRect(0, 0, width, height);
    return;
}

void PresetTableListBox::paintCell(juce::Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) {
    if(rowNumber < 0 || rowNumber >= _presets.size()) return;
    g.setFont(_font);
    const ProgramManager::PresetInfo &info = _presets.getReference(rowNumber);
    switch(columnId) {
        case 1:
            g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
            g.drawText(info.path, 4, 0, width - 8, height, juce::Justification::centredLeft, true);
            break;
        case 2:
            g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
            g.drawText(info.author, 4, 0, width - 8, height, juce::Justification::centredLeft, true);
            break;
        default:
            /* Do nothing */
            break;
    }
    return;
}

void PresetTableListBox::resized() {
    auto r = getLocalBounds();
    _table.setBounds(r);
    return;
}

void PresetTableListBox::updatePresetList() {
    _presets = ProgramManager::getPresetsInFolder(ProgramManager::userStateStoragePath());
    printf("Loaded %d presets\n", _presets.size());
    _table.updateContent();
    return;
}
