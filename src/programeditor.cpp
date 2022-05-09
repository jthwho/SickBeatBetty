#include "programeditor.h"

ProgramEditor::ProgramEditor(ProgramManager &pm) :
    _pm(pm),
    _model(pm),
    _table(&_model)
{
    addAndMakeVisible(_table);
    _pm.addListener(this);
}

ProgramEditor::~ProgramEditor() {
    _pm.removeListener(this);
}

void ProgramEditor::duplicateCurrentSelected() {
    int current = _table.getSelectedRow();
    _pm.duplicateProgram(current);
    return;
}

void ProgramEditor::resized() {
    auto r = getLocalBounds();
    _table.setBounds(r);
    return;
}

void ProgramEditor::programManagerListChanged() {
    _table.updateContent();
    return;
}

void ProgramEditor::programManagerProgramChanged(int index) {
    juce::ignoreUnused(index);
    _table.repaint();
    return;
}