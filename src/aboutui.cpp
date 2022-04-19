#include <BinaryData.h>
#include "aboutui.h"
#include "buildinfo.h"

constexpr char descHead[] = 
    "A MIDI Beat Generator\n"
    "Created by:\n"
    "Jason Howard & Anthony Smith";

AboutUI::AboutUI() :
    _pluginLink("Visit our plugin website", juce::URL("https://howardlogic.com"))
{
    const BuildInfo *buildInfo = getBuildInfo();
    juce::String desc = descHead;
    desc << "\n";
    desc << "\nVersion " << buildInfo->version;
    desc << "\nGIT " << buildInfo->repoident;
    desc << "\nBuilt " << buildInfo->date << " " << buildInfo->time << " (" << buildInfo->hostname << ")";

    juce::Image bettyImage = juce::ImageCache::getFromMemory(BinaryData::bettywhitedevilhorns_jpg, BinaryData::bettywhitedevilhorns_jpgSize);
    _betty.setImage(bettyImage, juce::RectanglePlacement::xMid | juce::RectanglePlacement::yMid);
    addAndMakeVisible(_betty);

    _nameLabel.setJustificationType(juce::Justification::left);
    _nameLabel.setText(buildInfo->name, juce::sendNotification);
    _nameLabel.setFont(juce::Font(30.0f, juce::Font::bold));
    addAndMakeVisible(_nameLabel);

    _pluginLink.setJustificationType(juce::Justification::left);
    addAndMakeVisible(_pluginLink);

    _descLabel.setText(desc, juce::sendNotification);
    _descLabel.setReadOnly(true);
    _descLabel.setMultiLine(true, true);
    addAndMakeVisible(_descLabel);
    
}

AboutUI::~AboutUI() {

}

void AboutUI::resized() {
    const int margin = 30;
    auto r = getLocalBounds();

    // Setup the margins
    r.removeFromTop(margin);
    r.removeFromLeft(margin);
    r.removeFromRight(margin);
    r.removeFromBottom(margin);

    _betty.setBounds(r.removeFromLeft(_betty.getImage().getWidth()));
    r.removeFromLeft(30);

    _nameLabel.setBounds(r.removeFromTop(50));
    _pluginLink.setBounds(r.removeFromTop(30));
    r.removeFromTop(10);
    _descLabel.setBounds(r);
    return;
}

void AboutUI::paint(juce::Graphics &g) {
    juce::ignoreUnused(g);
    return;
}
