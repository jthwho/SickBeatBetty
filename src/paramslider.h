#ifndef _PARAMSLIDER_H_
#define _PARAMSLIDER_H_

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#pragma once

class ParamSlider : public juce::Slider {
    public:
        ParamSlider(juce::RangedAudioParameter &param, juce::UndoManager *undoManager = nullptr);
        ~ParamSlider();

    private:
        juce::SliderParameterAttachment     _attach;
};

#endif
