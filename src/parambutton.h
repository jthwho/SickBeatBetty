#ifndef _PARAMBUTTON_H_
#define _PARAMBUTTON_H_
#pragma once

#include "paramhelper.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class ParamButton : public juce::ToggleButton {
    public:
        ParamButton(juce::RangedAudioParameter &param, juce::UndoManager *undoManager = nullptr);
        ~ParamButton();
        void resetToDefault();
        void setToRandomValue();

    private:
        juce::ButtonParameterAttachment     _attach;
        ParamHelper                         _paramHelper;
};

#endif
