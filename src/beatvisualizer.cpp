#include "beatvisualizer.h"

BeatVisualizer::BeatVisualizer() {

}

BeatVisualizer::~BeatVisualizer() {

}

void BeatVisualizer::setBeats(const BeatGen::BeatVector &val) {
    _beats = val;
    repaint();
    return;
}

void BeatVisualizer::setCurrentBeat(int val) {
    _currentBeat = val;
    repaint();
    return;
}

void BeatVisualizer::computeBeatLayout() {
    int bestSize = 0;
    float bestScore = 0.0;
    std::vector<juce::Point<int>> best;
    auto r = getLocalBounds();

    for(int size = 10; size < 50; size++) {
        int x = _margin;
        int y = _margin;
        bool fit = true;
        int line = 0;
        std::vector<int> beatsPerLine;
        std::vector<juce::Point<int>> points;
        beatsPerLine.push_back(0);
        for(int i = 0; i < _beats.size(); i++) {
            beatsPerLine[line]++;
            points.push_back(juce::Point<int>(x, y));
            x += size + _margin;
            if((x + size + _margin) >= r.getWidth()) {
                x = _margin;
                y += _margin + size;
                line++;
                beatsPerLine.push_back(0);
            }
            if((y + _margin + size) > r.getHeight()) {
                fit = false;
                break;
            }
        }
        if(!fit) break;
        int matchingLines = 0;
        int unmatchedLines = 0;
        for(int i = 1; i < beatsPerLine.size(); i++) {
            if(beatsPerLine[i] == beatsPerLine[0]) {
                matchingLines++;
            } else {
                if(beatsPerLine[i]) unmatchedLines++;
            }
        }
        float score = (float) matchingLines;
        score -= (float) unmatchedLines;

        // We want to find the best fit that:
        // 1. Uses the biggest size for the beat possible
        // 2. Has as many lines with the same number of beats as possible.
        if(score >= bestScore) {
            best = points;
            bestSize = size;
            bestScore = score;
        }
    
    }
    //printf("Beat Visualizer %p, choosing %d size for %d beats\n", this, bestSize, _beatCount);
    _beatSize = bestSize;
    _beatCoords = best;
    return;
}

void BeatVisualizer::paint(juce::Graphics &g) {
    if(_beatCoords.size() != _beats.size()) {
        computeBeatLayout();
    }
    g.setColour(juce::Colours::white);

    for(int i = 0; i < _beatCoords.size(); i++) {
        const juce::Point<int> &point = _beatCoords[i];
        double velocity = _beats[i].velocity;
        int x = point.getX(), y = point.getY();
        uint8_t level = velocity == 0.0 ? 0 : (uint8_t)(velocity * 200.0);
        g.setColour(juce::Colour(level, level, level));
        g.fillRect(x, y, _beatSize, _beatSize);

        juce::Colour boxColor = i == _currentBeat ? juce::Colours::red : juce::Colours::white;
        g.setColour(boxColor);
        g.drawRect(x, y, _beatSize, _beatSize);
    }
    return;
}

void BeatVisualizer::resized() {
    computeBeatLayout();
    return;
}