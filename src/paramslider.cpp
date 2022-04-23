#include "paramslider.h"
#include "paramhelper.h"

ParamSlider::ParamSlider(juce::RangedAudioParameter &param, juce::UndoManager *undoManager) :
    _attach(param, *this, undoManager), 
    _paramHelper(param)
{
    _attach.sendInitialUpdate();
}

ParamSlider::~ParamSlider() {

}

void ParamSlider::resetToDefault() {
    _paramHelper.resetToDefault();
}

void ParamSlider::setToRandomValue() {
    _paramHelper.setToRandomValue();
}