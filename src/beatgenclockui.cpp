#include "beatgenclockui.h"

BeatGenClockUI::BeatGenClockUI(BeatGen &beatGen, int clockIndex) :
    _beatGen(beatGen),
    _clockIndex(clockIndex),
    _rate(*beatGen.getParameter(BeatGen::ParamClockRate, clockIndex)->param()),
    _phaseOffset(*beatGen.getParameter(BeatGen::ParamClockPhaseOffset, clockIndex)->param()),
    _mixMode(*beatGen.getParameter(BeatGen::ParamClockMixMode, clockIndex)->param()),
    _level(*beatGen.getParameter(BeatGen::ParamClockLevel, clockIndex)->param())
{

}

BeatGenClockUI::~BeatGenClockUI() {
    _rate.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 30, 20);
    _rate.setSliderStyle(juce::Slider::LinearVertical);
    addAndMakeVisible(_rate);

    _phaseOffset.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 30, 20);
    _phaseOffset.setSliderStyle(juce::Slider::LinearVertical);
    addAndMakeVisible(_phaseOffset);

    _mixMode.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 30, 20);
    _mixMode.setSliderStyle(juce::Slider::LinearVertical);
    addAndMakeVisible(_mixMode);

    _level.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 30, 20);
    _level.setSliderStyle(juce::Slider::LinearVertical);
    addAndMakeVisible(_level);
}

void BeatGenClockUI::paint(juce::Graphics &g) {
    //g.fillAll(juce::Colours::white);
    //g.setColour(juce::Colours::black);
    //g.setFont(15.0f);
    g.drawFittedText("Clock 1", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
    return;
}

void BeatGenClockUI::resized() {
    auto r = getLocalBounds();
    printf("Resized %s\n", r.toString().toStdString().c_str());
    _rate.setBounds(r);
    //_phaseOffset.setBounds(r.removeFromLeft(30));
    //_mixMode.setBounds(r.removeFromLeft(30));
    //_level.setBounds(r.removeFromLeft(30));
    return;
}