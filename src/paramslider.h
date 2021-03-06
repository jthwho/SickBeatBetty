#ifndef _PARAMSLIDER_H_
#define _PARAMSLIDER_H_

#include "paramhelper.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#pragma once

class ParamSlider : public juce::Slider {
    public:
        ParamSlider(juce::RangedAudioParameter &param, juce::UndoManager *undoManager = nullptr);
        ~ParamSlider();

        ParamHelper& paramHelper();
        const ParamHelper& paramHelper() const;

    private:
        juce::SliderParameterAttachment     _attach;
        ParamHelper                         _paramHelper;

};

inline ParamHelper& ParamSlider::paramHelper() {
    return _paramHelper;
}

inline const ParamHelper& ParamSlider::paramHelper() const {
    return _paramHelper;
}

#endif
