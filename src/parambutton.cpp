#include "parambutton.h"
#include "paramhelper.h"

ParamButton::ParamButton(juce::RangedAudioParameter &param, juce::UndoManager *undoManager) :
    ToggleButton(),
    _attach(param, *this, undoManager),
    _paramHelper(param)
{
    _attach.sendInitialUpdate();
}

ParamButton::~ParamButton() {

}