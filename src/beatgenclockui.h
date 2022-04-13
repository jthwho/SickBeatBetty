#ifndef _BEATGENCLOCKUI_H_
#define _BEATGENCLOCKUI_H_
#pragma once

#include "beatgen.h"
#include "paramslider.h"

class BeatGenClockUI : public juce::Component {
    public:
        BeatGenClockUI(BeatGen &beatGen, int clockIndex);
        ~BeatGenClockUI();

    private:
        BeatGen         &_beatGen;
        int             _clockIndex;
        ParamSlider     _rate;
        ParamSlider     _phaseOffset;
        ParamSlider     _mixMode;
        ParamSlider     _level;

        void paint(juce::Graphics &g) override;
        void resized() override;
};

#endif
