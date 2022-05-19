#include "programtablelistboxmodel.h"

class EditableCell : public juce::Label {
    public:
        ProgramTableListBoxModel &model;
        int rowNumber;
        int columnId;

        EditableCell(ProgramTableListBoxModel &m) : model(m) {
            //setEditable(false, true, false);
            setInterceptsMouseClicks(false, false);
        }

        ~EditableCell() {

        }
};

ProgramTableListBoxModel::ProgramTableListBoxModel(ProgramManager &pm) : 
    TableListBoxModel(),
    _pm(pm)
{

}

ProgramTableListBoxModel::~ProgramTableListBoxModel() {

}

EditableCell *ProgramTableListBoxModel::findEditableCell(int rowNumber, int columnId) const {
    EditableCell *ret = nullptr;
    for(auto ec : _editableCells) {
        if(ec->rowNumber == rowNumber && ec->columnId == columnId) {
            ret = ec;
            break;
        }
    }
    return ret;
}

int ProgramTableListBoxModel::getNumRows() {
    return _pm.programCount();
}

void ProgramTableListBoxModel::paintRowBackground(juce::Graphics &g, int rowNumber, int width, int height, bool rowIsSelected) {
    juce::ignoreUnused(rowIsSelected);
    juce::Colour c = getLookAndFeel().findColour(juce::ListBox::backgroundColourId);
    if(rowNumber == _pm.currentProgram()) c = c.brighter();
    g.setColour(c);
    g.fillRect(0, 0, width, height);
    return;
}

void ProgramTableListBoxModel::paintCell(juce::Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) {
    juce::ignoreUnused(rowIsSelected);
    g.setFont(_font);
    switch(columnId) {
        case 1:
            if(rowNumber == _pm.currentProgram()) {
                juce::Colour c(0, 100, 0);
                juce::FillType ft;
                ft.setColour(c);
                g.setColour(c);
                g.setFillType(ft);
                g.fillRoundedRectangle(1.0f, 2.0f, (float)width - 2.0f, (float)height - 4.0f, 5.0f);
            }
            g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
            g.drawText(juce::String(rowNumber), 4, 0, width - 8, height, juce::Justification::centredLeft, true);
            break;
        case 2:
            //g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
            //g.drawText(_pm.programName(rowNumber), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
            break;
        default:
            /* Do nothing */
            break;
    }
    return;
}

void ProgramTableListBoxModel::startEdit(int rowNumber, int columnId) {
    EditableCell *ec = findEditableCell(rowNumber, columnId);
    if(ec != nullptr) ec->showEditor();
    return;
}

void ProgramTableListBoxModel::cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent &ev) {
    if(ev.mods.isLeftButtonDown()) {
        startEdit(rowNumber, columnId);
    }
    return;
}

void ProgramTableListBoxModel::cellClicked(int rowNumber, int columnId, const juce::MouseEvent &ev) {
    juce::ignoreUnused(columnId);
    if(ev.mods.isPopupMenu()) {
        juce::PopupMenu menu;
        menu.addItem("Rename", [this, rowNumber, columnId] {
            startEdit(rowNumber, columnId);
        });
        menu.addItem("Duplicate", [this, rowNumber] {
            _pm.duplicateProgram(rowNumber);
        });
        if(_pm.programCount() > 1) {
            menu.addItem("Delete", [this, rowNumber] {
                _pm.deleteProgram(rowNumber);
            });
        }
        menu.showMenuAsync(juce::PopupMenu::Options());

    } else if(ev.mods.isLeftButtonDown()) {
        _pm.changeProgram(rowNumber);
    }
    return;
}

juce::Component *ProgramTableListBoxModel::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component *existingComponentToUpdate) {
    juce::ignoreUnused(isRowSelected);
    juce::Component *ret = nullptr;
    switch(columnId) {
        case 2: {
            EditableCell *ec = dynamic_cast<EditableCell *>(existingComponentToUpdate);
            if(ec == NULL) {
                ec = new EditableCell(*this);
                _editableCells.add(ec);
            }
            ec->rowNumber = rowNumber;
            ec->columnId = columnId;
            ec->setText(_pm.programName(rowNumber), juce::dontSendNotification);
            ec->addListener(this);
            ret = ec;
        }
        break;

        default:
            /* Do Nothing */
            break;
    }
    return ret;
}

void ProgramTableListBoxModel::labelTextChanged(juce::Label *label) {
    EditableCell *ec = dynamic_cast<EditableCell *>(label);
    if(ec == nullptr) return;
    if(ec->columnId == 2) {
        _pm.renameProgram(ec->rowNumber, ec->getText());
    }
    return;
}