#include "paramhelper.h"
#include "stdlib.h"

ParamHelper::ParamHelper(juce::RangedAudioParameter& param) :
    _param(param)
{

}

void ParamHelper::resetToDefault() {
    float val = _param.getDefaultValue();
    updateParameter(val);    
}

void ParamHelper::setToRandomValue() {
    int intVal = rand() % 100 + 1;
    float val = intVal / 100;
    updateParameter(val);
}

void ParamHelper::updateParameter(float val) {
    _param.sendValueChangedMessageToListeners(val);
    _param.beginChangeGesture();
    _param.setValueNotifyingHost(val);
    _param.endChangeGesture();
    return;
}

ParamHelper::~ParamHelper() {

}
