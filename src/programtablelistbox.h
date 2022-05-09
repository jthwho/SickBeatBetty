#ifndef _PROGRAMTABLELISTBOX_H_
#define _PROGRAMTABLELISTBOX_H_
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class ProgramTableListBoxModel;

class ProgramTableListBox : public juce::TableListBox {
    public:
        ProgramTableListBox(ProgramTableListBoxModel *model = nullptr);
        ~ProgramTableListBox();

    private:

};

#endif /* _PROGRAMTABLELISTBOX_H_ not defined */
