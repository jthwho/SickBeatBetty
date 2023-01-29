#ifndef _ABOUTUI_H_
#define _ABOUTUI_H_
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class AboutUI : public juce::Component {
  public:
    AboutUI();
    ~AboutUI();

  private:
    juce::ImageComponent  _betty;
    juce::Label           _nameLabel;
    juce::HyperlinkButton _pluginLink;
    juce::TextEditor      _descLabel;

    void resized() override;
    void paint(juce::Graphics & g) override;
};

#endif
