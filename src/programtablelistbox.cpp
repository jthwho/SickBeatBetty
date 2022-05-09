#include "programtablelistbox.h"
#include "programtablelistboxmodel.h"

ProgramTableListBox::ProgramTableListBox(ProgramTableListBoxModel *model) :
    TableListBox(juce::String(), model)
{
    getHeader().addColumn("ID", 0, 30, 30, 30, 31);
    getHeader().addColumn("Name", 1, 150, 100, -1, 31);
    getHeader().setStretchToFitActive(true);
}

ProgramTableListBox::~ProgramTableListBox() {

}
