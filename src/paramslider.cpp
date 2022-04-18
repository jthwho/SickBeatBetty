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
    _param.setValue(_param.getDefaultValue());     
    return; 
}