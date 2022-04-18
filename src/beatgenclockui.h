#ifndef _BEATGENCLOCKUI_H_
#define _BEATGENCLOCKUI_H_
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "beatgen.h"
#include "paramslider.h"
#include "parambutton.h"

class BeatGenClockUI : public juce::Component {
    public:
        BeatGenClockUI(BeatGen &beatGen, int clockIndex);
        ~BeatGenClockUI();
        void resetToDefaults();

    private:
        BeatGen             &_beatGen;
        int                 _clockIndex;
        juce::ImageButton   _reset;
        ParamButton         _enabled;
        juce::Label         _rateLabel;
        ParamSlider         _rate;
        juce::Label         _phaseOffsetLabel;
        ParamSlider         _phaseOffset;
        juce::Label         _mixModeLabel;
        ParamSlider         _mixMode;
        juce::Label         _levelLabel;
        ParamSlider         _level;

        void paint(juce::Graphics &g) override;
        void resized() override;
};

#endif
