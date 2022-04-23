#ifndef _PARAMCOMBOBOX_H_
#define _PARAMCOMBOBOX_H_
#pragma once

#include "juce_gui_basics/juce_gui_basics.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "paramhelper.h"

class ParamComboBox : public juce::ComboBox {
    public:
        ParamComboBox(juce::RangedAudioParameter &param, juce::UndoManager *undoManager = nullptr);
        ~ParamComboBox();

        ParamHelper &paramHelper();
        const ParamHelper &paramHelper() const;

    private:
        juce::ComboBoxParameterAttachment   _attach;
        ParamHelper                         _paramHelper;
};

inline ParamHelper &ParamComboBox::paramHelper() {
    return _paramHelper;
}

inline const ParamHelper &ParamComboBox::paramHelper() const {
    return _paramHelper;
}

#endif
