
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
    float val = _param.getDefaultValue();
    printf("Resetting %s to %f\n", _param.name.toStdString().c_str(), val);
    _param.sendValueChangedMessageToListeners(val);
    _param.beginChangeGesture();
    _param.setValueNotifyingHost(val);
    _param.endChangeGesture();
    return;
}

