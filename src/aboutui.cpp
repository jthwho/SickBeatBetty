#include <BinaryData.h>
#include "aboutui.h"

constexpr char desc[] = 
    "A MIDI Beat Generator\n"
    "Created by:\n"
    "Jason Howard & Anthony Smith";

AboutUI::AboutUI() :
    _nameLabel("NameLabel", JucePlugin_Name),
    _versionLabel("VersionLabel", "Version " JucePlugin_VersionString),
    _descLabel("DescLabel", desc),
    _pluginLink("Visit our plugin website", juce::URL("https://howardlogic.com"))
{
    juce::Image bettyImage = juce::ImageCache::getFromMemory(BinaryData::bettywhitedevilhorns_jpg, BinaryData::bettywhitedevilhorns_jpgSize);
    _betty.setImage(bettyImage, juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yTop);
    addAndMakeVisible(_betty);
    _nameLabel.setJustificationType(juce::Justification::left);
    addAndMakeVisible(_nameLabel);
    _versionLabel.setJustificationType(juce::Justification::left);
    addAndMakeVisible(_versionLabel);
    _descLabel.setJustificationType(juce::Justification::left);
    addAndMakeVisible(_descLabel);
    _pluginLink.setJustificationType(juce::Justification::left);
    addAndMakeVisible(_pluginLink);
}

AboutUI::~AboutUI() {

}

void AboutUI::resized() {
    auto r = getLocalBounds();
    // Add a bit of margin.
    // FIXME: And some point, we'll want to put maybe a logo on the left
    r.removeFromTop(30);
    r.removeFromLeft(30);
    _betty.setBounds(r.removeFromLeft(_betty.getImage().getWidth()));
    r.removeFromLeft(30);

    _nameLabel.setBounds(r.removeFromTop(30));
    _versionLabel.setBounds(r.removeFromTop(30));
    _pluginLink.setBounds(r.removeFromTop(30));
    _descLabel.setBounds(r);
    return;
}

void AboutUI::paint(juce::Graphics &g) {
    juce::ignoreUnused(g);
    return;
}
