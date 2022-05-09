#ifndef _PROGRAMTABLELISTBOXMODEL_H_
#define _PROGRAMTABLELISTBOXMODEL_H_
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "programmanager.h"

class EditableCell;

class ProgramTableListBoxModel :
    public juce::Component,
    public juce::TableListBoxModel,
    public juce::Label::Listener
{
    public:
        ProgramTableListBoxModel(ProgramManager &pm);
        ~ProgramTableListBoxModel();

        int getNumRows() override;
        void paintRowBackground(juce::Graphics &, int rowNumber, int width, int height, bool rowIsSelected) override;
        void paintCell(juce::Graphics &, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
        void cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent &ev) override;
        void cellClicked(int rowNumber, int columnId, const juce::MouseEvent &ev) override;
        juce::Component *refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, juce::Component *existingComponentToUpdate) override;

    private:
        ProgramManager                  &_pm;
        juce::Font                      _font { 14.0f };
        juce::Array<EditableCell *>     _editableCells;

        void labelTextChanged(juce::Label *label) override;
        EditableCell *findEditableCellForRow(int rowNumber) const;

};

#endif /* _PROGRAMTABLELISTBOXMODEL_H_ not defined */
