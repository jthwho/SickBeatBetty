
#include "parambutton.h"

ParamButton::ParamButton(juce::RangedAudioParameter &param, juce::UndoManager *undoManager) :
    ToggleButton(),
    _attach(param, *this, undoManager),
    _param(param)
{
    _attach.sendInitialUpdate();
}

ParamButton::~ParamButton() {

}

void ParamButton::resetToDefault() {
    _param.setValue(_param.getDefaultValue());
    return;
}

