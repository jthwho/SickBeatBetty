#include "paramslider.h"

ParamSlider::ParamSlider(juce::RangedAudioParameter &param, juce::UndoManager *undoManager) :
    _attach(param, *this, undoManager)
{
    _attach.sendInitialUpdate();
}

ParamSlider::~ParamSlider() {

}
