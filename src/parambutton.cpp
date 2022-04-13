
#include "parambutton.h"

ParamButton::ParamButton(juce::RangedAudioParameter &param, juce::UndoManager *undoManager) :
    ToggleButton(),
    _attach(param, *this, undoManager)
{
    _attach.sendInitialUpdate();
}

ParamButton::~ParamButton() {

}
