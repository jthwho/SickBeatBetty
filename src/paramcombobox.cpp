#include "paramcombobox.h"

ParamComboBox::ParamComboBox(juce::RangedAudioParameter &param, juce::UndoManager *undoManager) :
    ComboBox(),
    _attach(param, *this, undoManager),
    _paramHelper(param)
{
    auto *c = dynamic_cast<juce::AudioParameterChoice *>(&param);
    if(c != nullptr) {
        addItemList(c->getAllValueStrings(), 1);
    }
    _attach.sendInitialUpdate();
}

ParamComboBox::~ParamComboBox() {

}

