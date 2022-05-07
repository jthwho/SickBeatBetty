#include "valuetreetexteditor.h"

ValueTreeTextEditor::ValueTreeTextEditor(juce::ValueTree &vt, const juce::Identifier &id) :
    juce::TextEditor(),
    _valueTree(vt),
    _id(id)
{
    onTextChange = [this]{
        auto text = getText();
        auto value = _valueTree[_id].toString();
        if(text != value) _valueTree.setProperty(_id, text, nullptr);
    };
}

ValueTreeTextEditor::~ValueTreeTextEditor()
{

}

void ValueTreeTextEditor::valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &id) {
    if(tree == _valueTree && id == _id) updateFromValueTree();
    return;
}

void ValueTreeTextEditor::updateFromValueTree() {
    juce::String text = _valueTree[_id].toString();
    if(text != getText()) setText(text, true);
    return;
}


