#ifndef _BEATGENUI_H_
#define _BEATGENUI_H_
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "beatgen.h"
#include "beatgenclockui.h"
#include "parambutton.h"
#include "paramslider.h"

class BeatGenUI : public juce::Component {
    public:
        BeatGenUI(BeatGen &beatGen);
        ~BeatGenUI();

    private:
        BeatGen                             &_beatGen;
        ParamButton                         _enabled;
        juce::Label                         _labelNote;
        ParamSlider                         _note;
        juce::Label                         _labelSteps;
        ParamSlider                         _steps;
        juce::Label                         _labelPhaseOffset;
        ParamSlider                         _phaseOffset;
        juce::Label                         _labelBars;
        ParamSlider                         _bars;
        juce::Label                         _labelLevel;
        ParamSlider                         _level;
        juce::OwnedArray<BeatGenClockUI>    _clocks;

        void paint(juce::Graphics &g) override;
        void resized() override;
};

#endif
