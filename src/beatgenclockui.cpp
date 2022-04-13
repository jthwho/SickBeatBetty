#include "beatgenclockui.h"

#define LABEL_HEIGHT        30
#define SLIDER_HEIGHT_MIN   50
#define SLIDER_WIDTH_MIN    50
#define TEXTBOX_HEIGHT_MIN  20

BeatGenClockUI::BeatGenClockUI(BeatGen &beatGen, int clockIndex) :
    _beatGen(beatGen),
    _clockIndex(clockIndex),
    _enabled(*beatGen.getParameter(BeatGen::ParamClockEnabled, clockIndex)->param()),
    _rate(*beatGen.getParameter(BeatGen::ParamClockRate, clockIndex)->param()),
    _phaseOffset(*beatGen.getParameter(BeatGen::ParamClockPhaseOffset, clockIndex)->param()),
    _mixMode(*beatGen.getParameter(BeatGen::ParamClockMixMode, clockIndex)->param()),
    _level(*beatGen.getParameter(BeatGen::ParamClockLevel, clockIndex)->param())
{
    _enabled.setButtonText("Euclid Enabled");
    _enabled.setTooltip("When enabled, this clock will be mixed with the previous clock to make a euclidian beat");
    _enabled.setToggleable(true);
    addAndMakeVisible(_enabled);

    _rateLabel.setText("Rate", juce::dontSendNotification);
    _rateLabel.setJustificationType(juce::Justification::centred);
    _rateLabel.setTooltip("Total number of ticks in the generation period");
    addAndMakeVisible(_rateLabel);

    _rate.setTextBoxStyle(juce::Slider::TextBoxAbove, false, SLIDER_WIDTH_MIN, TEXTBOX_HEIGHT_MIN);
    _rate.setSliderStyle(juce::Slider::LinearVertical);
    addAndMakeVisible(_rate);

    _phaseOffsetLabel.setText("Phase", juce::dontSendNotification);
    _phaseOffsetLabel.setJustificationType(juce::Justification::centred);
    _phaseOffsetLabel.setTooltip("Adjusts where the first tick starts within the generation period");
    addAndMakeVisible(_phaseOffsetLabel);

    _phaseOffset.setTextBoxStyle(juce::Slider::TextBoxAbove, false, SLIDER_WIDTH_MIN, TEXTBOX_HEIGHT_MIN);
    _phaseOffset.setSliderStyle(juce::Slider::LinearVertical);
    addAndMakeVisible(_phaseOffset);

    _mixModeLabel.setText("Mix Mode", juce::dontSendNotification);
    _mixModeLabel.setJustificationType(juce::Justification::centred);
    _mixModeLabel.setTooltip("When used for Euclidian generation, this is the mode used to mix with the previous clock");
    addAndMakeVisible(_mixModeLabel);

    _mixMode.setTextBoxStyle(juce::Slider::TextBoxAbove, false, SLIDER_WIDTH_MIN, TEXTBOX_HEIGHT_MIN);
    _mixMode.setSliderStyle(juce::Slider::LinearVertical);
    addAndMakeVisible(_mixMode);

    _levelLabel.setText("Level", juce::dontSendNotification);
    _levelLabel.setJustificationType(juce::Justification::centred);
    _levelLabel.setTooltip("Amount this clock should contribute to the velocity of a beat");
    addAndMakeVisible(_levelLabel);

    _level.setTextBoxStyle(juce::Slider::TextBoxAbove, false, SLIDER_WIDTH_MIN, TEXTBOX_HEIGHT_MIN);
    _level.setSliderStyle(juce::Slider::LinearVertical);
    addAndMakeVisible(_level);

}

BeatGenClockUI::~BeatGenClockUI() {

}

void BeatGenClockUI::paint(juce::Graphics &g) {
    g.fillAll(juce::Colour(32, 32, 32).brighter(_clockIndex % 2 ? 0.03f : 0.0f));
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawFittedText(
        juce::String::formatted("Clock %d", _clockIndex + 1),
        0, 0, getWidth(), LABEL_HEIGHT, juce::Justification::centred, 1
    );
    return;
}

void BeatGenClockUI::resized() {
    auto r = getLocalBounds();
    r.removeFromTop(LABEL_HEIGHT);
    _enabled.setBounds(r.removeFromTop(30));

    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    using Px = juce::Grid::Px;
    using Item = juce::GridItem;

    grid.templateRows = { Track(Px(LABEL_HEIGHT)), Track(Fr(1)) };
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    grid.items = { 
        Item(_levelLabel), Item(_rateLabel), Item(_phaseOffsetLabel), Item(_mixModeLabel),
        Item(_level), Item(_rate), Item(_phaseOffset), Item(_mixMode) 
    };
    grid.performLayout(r);

    /*
    juce::FlexBox fb(
        juce::FlexBox::Direction::row,
        juce::FlexBox::Wrap::noWrap,
        juce::FlexBox::AlignContent::stretch,
        juce::FlexBox::AlignItems::stretch,
        juce::FlexBox::JustifyContent::center
    );
    fb.items.add(juce::FlexItem(_rate).withMinWidth(SLIDER_WIDTH_MIN).withMinHeight(SLIDER_HEIGHT_MIN));
    fb.items.add(juce::FlexItem(_phaseOffset).withMinWidth(SLIDER_WIDTH_MIN).withMinHeight(SLIDER_HEIGHT_MIN));
    fb.items.add(juce::FlexItem(_mixMode).withMinWidth(SLIDER_WIDTH_MIN).withMinHeight(SLIDER_HEIGHT_MIN));
    fb.items.add(juce::FlexItem(_level).withMinWidth(SLIDER_WIDTH_MIN).withMinHeight(SLIDER_HEIGHT_MIN));
    fb.performLayout(r.toFloat());
    */

    return;
}
