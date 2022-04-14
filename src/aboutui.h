#ifndef _ABOUTUI_H_
#define _ABOUTUI_H_
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class AboutUI : public juce::Component {
    public:
        AboutUI();
        ~AboutUI();

    private:
        juce::Label                 _nameLabel;
        juce::Label                 _versionLabel;
        juce::Label                 _descLabel;
        juce::HyperlinkButton       _pluginLink;

        void resized() override;
        void paint(juce::Graphics &g) override;
};

#endif
