#ifndef _VALUETREETEXTEDITOR_H_
#define _VALUETREETEXTEDITOR_H_
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>

class ValueTreeTextEditor :
    public juce::TextEditor,
    public juce::ValueTree::Listener
{
    public:
        ValueTreeTextEditor(juce::ValueTree &valueTree, const juce::Identifier &id);
        ~ValueTreeTextEditor();

        void updateFromValueTree();

    private:
        juce::ValueTree     &_valueTree;
        juce::Identifier    _id;

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged, const juce::Identifier &property);


};

#endif
