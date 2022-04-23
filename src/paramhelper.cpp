#include "paramhelper.h"
//#include <juce_audio_processors/juce_audio_processors.h>

ParamHelper::ParamHelper(juce::RangedAudioParameter& param) :
    _param(param)
{

}

void ParamHelper::resetToDefault() {
    float val = _param.getDefaultValue();
    updateParameter(val);    
}

void ParamHelper::setToRandomValue() {
    juce::Random rand = juce::Random();
    float val = rand.nextFloat();
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
