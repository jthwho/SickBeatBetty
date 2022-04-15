#include <juce_core/juce_core.h>
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

struct Layout {
    int choice = 0;
    int totalLines = 0;
    int beatsPerLine = 0;
    int beatsPerLastLine = 0;
    int size;
};

void BeatVisualizer::computeBeatLayout() {
     _beatCoords.clear();
    _beatSize = 0;
    if(!_beats.size()) return;

    std::vector<Layout> possibleLayouts;

    auto r = getLocalBounds();
    int maxLines = 8; // FIXME: Make this decision based on the bounds.
    int minSize = 13;
    int beats = (int)_beats.size();
    int width = r.getWidth() - (_margin * 2);
    int height = r.getHeight() - (_margin * 2);

    // Compute all the possible layout options.
    for(int i = 1; i <= maxLines; i++) {
        int beatsPerLine = beats / i;
        int beatsPerLastLine = beats % i;
        int totalLines = i;
        if(beatsPerLastLine == 0) {
            beatsPerLastLine = beatsPerLine;
        } else {
            totalLines++;
        }
        if(beatsPerLine == 0) continue;
        int wsize = width / beatsPerLine;
        int hsize = height / totalLines;
        int size = juce::jmin(wsize, hsize);
        if(size < minSize) continue; // Beat box would be too small, keep looking.
        Layout l;
        l.choice = i;
        l.totalLines = totalLines;
        l.beatsPerLine = beatsPerLine;
        l.beatsPerLastLine = beatsPerLastLine;
        l.size = size;
        possibleLayouts.push_back(l);
    }
    printf("%d beats in (%d, %d) has %d options\n", beats, width, height, (int)possibleLayouts.size());

    int choice = -1;
    // First, try to find the largest size where there's no short line.
    if(choice == -1) {
        std::vector<int> matches;
        for(int i = 0; i < possibleLayouts.size(); i++) {
            const Layout &l = possibleLayouts.at(i);
            if(l.totalLines > 1 && 
                l.beatsPerLine > 1 && 
                l.beatsPerLine == l.beatsPerLastLine) matches.push_back(i);
        }
        int size = 0;
        for(int i = 0; i < matches.size(); i++) {
            const Layout &l = possibleLayouts.at(matches.at(i));
            if(l.size > size) {
                size = l.size;
                choice = matches.at(i);
            }
        }
    }
    // Ok, if we didn't find an 
    if(choice == -1) {
        int size = 0;
        for(int i = 0; i < possibleLayouts.size(); i++) {
            const Layout &l = possibleLayouts[i];
            if(l.size > size) {
                choice = i;
                size = l.size;
            }
        }
    }
    if(choice == -1) {
        printf("Failed to find a choice!\n");
        return;
    }

    const Layout &l = possibleLayouts.at(choice);
    printf("Best %d: %d size, %d lines, %d bpl %d last\n", choice, l.size, l.totalLines, l.beatsPerLine, l.beatsPerLastLine);
    int y = _margin;
    int lastLine = l.totalLines - 1;
    _beatSize = l.size - _margin;
    for(int line = 0; line < l.totalLines; line++) {
        int totalBeats = (line == lastLine) ? l.beatsPerLastLine : l.beatsPerLine;
        int x = _margin;
        for(int beat = 0; beat < totalBeats; beat++) {
            _beatCoords.push_back(juce::Point<int>(x, y));
            x += l.size;
        }
        y += l.size;
    }
    return;
}

/*
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
*/

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