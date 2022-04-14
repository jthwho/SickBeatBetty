#ifndef _BEATVISUALIZER_H_
#define _BEATVISUALIZER_H_
#pragma once

#include <vector>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "beatgen.h"

class BeatVisualizer : public juce::Component {
    public:
        BeatVisualizer();
        ~BeatVisualizer();

        void setBeats(const BeatGen::BeatVector &val);
        void setCurrentBeat(int val);

    private:
        int                             _beatSize = 20;
        int                             _margin = 5;
        int                             _currentBeat = 0;
        std::vector<juce::Point<int>>   _beatCoords;
        BeatGen::BeatVector             _beats;

        void computeBeatLayout();

        void paint(juce::Graphics &g) override;
        void resized() override;
};

#endif
