#ifndef _PROGRAMEDITOR_H_
#define _PROGRAMEDITOR_H_
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "programmanager.h"
#include "programtablelistbox.h"
#include "programtablelistboxmodel.h"

class ProgramEditor : 
    public juce::Component,
    public ProgramManager::Listener
{
    public:
        ProgramEditor(ProgramManager &pm);
        ~ProgramEditor();

        void resized() override;
        void duplicateCurrentSelected();

    private:
        ProgramManager              &_pm;
        ProgramTableListBoxModel    _model;
        ProgramTableListBox         _table;

        void programManagerListChanged() override;
        void programManagerProgramChanged(int index) override;
};

#endif /* _PROGRAMEDITOR_H_ not defined */
