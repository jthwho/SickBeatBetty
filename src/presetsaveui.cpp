#include "presetsaveui.h"

PresetSaveUI::PresetSaveUI(PluginProcessor &p) :
    juce::Component("PresetSaveUI"),
    _proc(p),
    _nameLabel("NameLabel", "Name"),
    _name(p.propsValueTree(), "name"),
    _authorLabel("AuthorLabel", "Author"),
    _author(p.propsValueTree(), "author"),
    _descLabel("DescLabel", "Description"),
    _desc(p.propsValueTree(), "desc"),
    _saveButton("Save"),
    _cancelButton("Cancel")
{
    _saveButton.onClick = [this]{
        save();
    };
    _cancelButton.onClick = [this]{
        cancel();
    };
    
    addAndMakeVisible(_nameLabel);
    addAndMakeVisible(_name);
    addAndMakeVisible(_authorLabel);
    addAndMakeVisible(_author);
    addAndMakeVisible(_descLabel);
    addAndMakeVisible(_desc);
    addAndMakeVisible(_saveButton);
    addAndMakeVisible(_cancelButton);

    setSize(400, 300);

    _desc.setMultiLine(true, true);
    _desc.setReturnKeyStartsNewLine(true);
    _name.updateFromValueTree();
    _author.updateFromValueTree();
    _desc.updateFromValueTree();
}

PresetSaveUI::~PresetSaveUI() {

}

void PresetSaveUI::paint(juce::Graphics &g) {
    g.fillAll(juce::Colour(32, 32, 32));
    return;
}

void PresetSaveUI::resized() {
    auto r = getLocalBounds();

    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    using Px = juce::Grid::Px;
    using Item = juce::GridItem;

    grid.templateRows = { Track(Fr(1)), Track(Fr(1)) };
    grid.templateColumns = { Track(Px(60)), Track(Fr(1)) };
    grid.items = { 
        Item(_nameLabel), Item(_name),
        Item(_authorLabel), Item(_author)
    };
    grid.performLayout(r.removeFromTop(60));

    grid.templateRows = { Track(Fr(1)) };
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)) };
    grid.items = { Item(_saveButton), Item(_cancelButton) };
    grid.performLayout(r.removeFromBottom(30));
    _descLabel.setBounds(r.removeFromTop(30));
    _desc.setBounds(r);
    return;
}

void PresetSaveUI::save() {
    return;
}

void PresetSaveUI::cancel() {
    return;
}

