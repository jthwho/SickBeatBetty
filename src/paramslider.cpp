#include "paramslider.h"

ParamSlider::ParamSlider(juce::RangedAudioParameter &param, juce::UndoManager *undoManager) :
    _attach(param, *this, undoManager), 
    _param(param)
{
    _attach.sendInitialUpdate();
}

ParamSlider::~ParamSlider() {

}

void ParamSlider::resetToDefault() {
    float val = _param.getDefaultValue();
    printf("Resetting %s to %f\n", _param.name.toStdString().c_str(), val);
    _param.sendValueChangedMessageToListeners(val);
    _param.beginChangeGesture();
    _param.setValueNotifyingHost(val);
    _param.endChangeGesture();
    return; 
}