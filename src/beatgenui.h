#ifndef _BEATGENUI_H_
#define _BEATGENUI_H_
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "beatgen.h"
#include "beatgenclockui.h"
#include "parambutton.h"
#include "paramslider.h"
#include "beatvisualizer.h"

class BeatGenUI : 
    public juce::Component, 
    public juce::ActionListener
{
    public:
        BeatGenUI(BeatGen &beatGen);
        ~BeatGenUI();

    private:
        BeatGen                             &_beatGen;
        BeatVisualizer                      _beatVisualizer;
        ParamButton                         _enabled;
        ParamButton                         _solo;
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
        juce::Label                         _labelSwing;
        ParamSlider                         _swing;
        juce::OwnedArray<BeatGenClockUI>    _clocks;

        void paint(juce::Graphics &g) override;
        void resized() override;
        void actionListenerCallback(const juce::String &msg) override;

};

#endif
