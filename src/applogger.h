#ifndef _APPLOGGER_H_
#define _APPLOGGER_H_
#pragma once

#include <juce_core/juce_core.h>

class AppLogger : public juce::Logger {
    public:
        AppLogger(const juce::String &logname);
        ~AppLogger();

    protected:
        void logMessage(const juce::String &msg) override;

    private:
        std::unique_ptr<juce::FileOutputStream>     _stream;

        void setup(const juce::String &logFileName);
};

#endif
