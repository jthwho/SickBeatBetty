#include "paramhelper.h"

ParamHelper::ParamHelper(juce::RangedAudioParameter& param) :
    _param(param)
{

}

void ParamHelper::resetToDefault() {
    float val = _param.getDefaultValue();
    printf("Resetting %s to %f\n", _param.name.toStdString().c_str(), val);
    _param.sendValueChangedMessageToListeners(val);
    _param.beginChangeGesture();
    _param.setValueNotifyingHost(val);
    _param.endChangeGesture();
    return;
}

void ParamHelper::setToRandomValues() {
    //TODO:  Figure this out.
}

ParamHelper::~ParamHelper() {

}
