#ifndef _PARAMHELPER_H_
#define _PARAMHELPER_H_
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class ParamHelper {

    public:
        ParamHelper(juce::RangedAudioParameter& param);
        ~ParamHelper();
        void resetToDefault();
        void setToRandomValue();

    private:
        juce::RangedAudioParameter &_param;
        void updateParameter(float val);

};
#endif