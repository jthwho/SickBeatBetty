#include "paramhelper.h"

ParamHelper::ParamHelper(juce::RangedAudioParameter& param) :
    _param(param)
{

}

void ParamHelper::resetToDefault() {
    updateParameter(_param.getDefaultValue());
}

void ParamHelper::setToRandomValue() {
    updateParameter(juce::Random::getSystemRandom().nextFloat());
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
