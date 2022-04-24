#include <BinaryData.h>
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
    juce::Image resetImage = juce::ImageCache::getFromMemory(BinaryData::reload_png, BinaryData::reload_pngSize);

    _reset.setImages(false, true, true, 
        resetImage, 1.0, juce::Colours::white,
        resetImage, 1.0, juce::Colours::blueviolet,
        resetImage, 1.0, juce::Colours::yellow
    );
    _reset.setTooltip(juce::String::formatted("Resets clock group %d back to default values", _clockIndex + 1));
    _reset.onClick = [this] { resetToDefaults(); }; 
    addAndMakeVisible(_reset);

    _randomize.setButtonText("Rand");
    _randomize.onClick = [this] { randomizeValues(); };
    addAndMakeVisible(_randomize);

    _enabled.setButtonText("Euclid Enabled");
    _enabled.setTooltip("When enabled, this clock will be mixed with the previous clock to make a euclidian beat");
    _enabled.setToggleable(true);
    addAndMakeVisible(_enabled);

    _rateLabel.setText("Rate", juce::dontSendNotification);
    _rateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(_rateLabel);

    _rate.setTextBoxStyle(juce::Slider::TextBoxAbove, false, SLIDER_WIDTH_MIN, TEXTBOX_HEIGHT_MIN);
    _rate.setSliderStyle(juce::Slider::LinearVertical);
    _rate.setTooltip("Total number of ticks in the generation period");
    addAndMakeVisible(_rate);

    _phaseOffsetLabel.setText("Phase", juce::dontSendNotification);
    _phaseOffsetLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(_phaseOffsetLabel);

    _phaseOffset.setTextBoxStyle(juce::Slider::TextBoxAbove, false, SLIDER_WIDTH_MIN, TEXTBOX_HEIGHT_MIN);
    _phaseOffset.setSliderStyle(juce::Slider::LinearVertical);
    _phaseOffset.setTooltip("Adjusts where the first tick starts within the generation period");
    addAndMakeVisible(_phaseOffset);

    _mixModeLabel.setText("Mix Mode", juce::dontSendNotification);
    _mixModeLabel.setJustificationType(juce::Justification::left);
    addAndMakeVisible(_mixModeLabel);

    //_mixMode.setTextBoxStyle(juce::Slider::TextBoxAbove, false, SLIDER_WIDTH_MIN, TEXTBOX_HEIGHT_MIN);
    //_mixMode.setSliderStyle(juce::Slider::LinearVertical);
    _mixMode.setTooltip("When used for Euclidian generation, this is the mode used to mix with the previous clock");
    addAndMakeVisible(_mixMode);

    _levelLabel.setText("Level", juce::dontSendNotification);
    _levelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(_levelLabel);

    _level.setTextBoxStyle(juce::Slider::TextBoxAbove, false, SLIDER_WIDTH_MIN, TEXTBOX_HEIGHT_MIN);
    _level.setSliderStyle(juce::Slider::LinearVertical);
    _level.setTooltip("Amount this clock should contribute to the velocity of a beat");
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

void BeatGenClockUI::resetToDefaults(){
    _enabled.paramHelper().resetToDefault();
    _rate.paramHelper().resetToDefault();
    _phaseOffset.paramHelper().resetToDefault();
    _mixMode.paramHelper().resetToDefault();
    _level.paramHelper().resetToDefault();
}

void BeatGenClockUI::randomizeValues() {
    _rate.paramHelper().setToRandomValue();
    _phaseOffset.paramHelper().setToRandomValue();
    _level.paramHelper().setToRandomValue();
}

void BeatGenClockUI::resized() {
    auto r = getLocalBounds();
    auto r2 = r.removeFromTop(LABEL_HEIGHT);
    _reset.setBounds(r2.removeFromRight(40));
    _randomize.setBounds(r2.removeFromLeft(40));
    _enabled.setBounds(r.removeFromTop(30));
    
    auto r3 = r.removeFromBottom(30);
    _mixModeLabel.setBounds(r3.removeFromLeft(80));
    r3.removeFromRight(10);
    _mixMode.setBounds(r3);

    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    using Px = juce::Grid::Px;
    using Item = juce::GridItem;

    grid.templateRows = { Track(Px(LABEL_HEIGHT)), Track(Fr(1)) };
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    grid.items = { 
        Item(_levelLabel), Item(_rateLabel), Item(_phaseOffsetLabel),
        Item(_level), Item(_rate), Item(_phaseOffset)
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
