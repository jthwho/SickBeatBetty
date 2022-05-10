#include "presetsaveui.h"

const juce::Identifier PresetNameIdentifier("PresetName");
const juce::Identifier PresetAuthorIdentifier("PresetAuthor");
const juce::Identifier PresetDescIdentifier("PresetDesc");

PresetSaveUI::PresetSaveUI(PluginProcessor &p) :
    juce::Component("PresetSaveUI"),
    _proc(p),
    _nameLabel("NameLabel", "Name"),
    _name(p.programManager().appState(), PresetNameIdentifier),
    _authorLabel("AuthorLabel", "Author"),
    _author(p.programManager().appState(), PresetAuthorIdentifier),
    _descLabel("DescLabel", "Description"),
    _desc(p.programManager().appState(), PresetDescIdentifier),
    _saveButton("Save"),
    _cancelButton("Cancel")
{
    _saveButton.onClick = [this] {
        save();
    };
    _cancelButton.onClick = [this] {
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

void PresetSaveUI::save(bool replace) {
    juce::String filename = _proc.programManager().appState().getProperty(PresetNameIdentifier).toString();
    filename = juce::File::createLegalFileName(filename);
    if(filename.isEmpty()) {
        juce::NativeMessageBox::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "Invalid Name", "Can't save, preset name isn't valid", this, nullptr
        );
        return;
    }
    filename += ".preset";
    juce::File file = ProgramManager::userStateStoragePath().getChildFile(filename);
    if(!replace && file.exists()) {
        juce::NativeMessageBox::showYesNoBox(
            juce::MessageBoxIconType::QuestionIcon,
            "Preset Exists",
            "This preset already exists, do you want to replace it?",
            this,
            juce::ModalCallbackFunction::create([this](int ret){
                if(ret == 1) save(true);
            })
        );
        return;
    }
    
    juce::FileOutputStream stream(file);
    auto status = stream.getStatus();
    if(status.failed()) {
        juce::NativeMessageBox::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "Preset Save Failed", status.getErrorMessage(), 
            this, nullptr
        );
        juce::Logger::writeToLog("Failed to save preset '" + file.getFullPathName() + "': " + status.getErrorMessage());
        return;
    }

    auto state = _proc.programManager().getStateXML();
    if(state == nullptr) {
        juce::NativeMessageBox::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "Preset Save Failed", "Unable to get preset state", 
            this, nullptr
        );
        juce::Logger::writeToLog("Failed to save preset '" + file.getFullPathName() + "': Unable to get preset state");
        return;
    }

    bool success = stream.writeText(state->toString(), false, false, nullptr);
    if(!success) {
       juce::NativeMessageBox::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "Preset Save Failed", "Failed to write state data to file", 
            this, nullptr
        );
        juce::Logger::writeToLog("Failed to save preset '" + file.getFullPathName() + "': Failed to write state data");
        return;
    }
    stream.flush();
    juce::Logger::writeToLog("Wrote preset " + file.getFullPathName());
    closeDialog(0);
    return;
}

void PresetSaveUI::cancel() {
    closeDialog(1);
    return;
}

void PresetSaveUI::closeDialog(int ret) {
    juce::DialogWindow* dw = findParentComponentOfClass<juce::DialogWindow>();
    if(dw != nullptr) dw->exitModalState(ret);
    return;
}
