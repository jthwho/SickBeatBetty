#ifndef _APPLOGGER_H_
#define _APPLOGGER_H_
#pragma once

#include <juce_core/juce_core.h>

class AppLogger : public juce::Logger {
    public:
        // Returns the singleton instance of the AppLogger
        static AppLogger &instance();

        ~AppLogger();

    protected:
        void logMessage(const juce::String &msg) override;

    private:
        AppLogger(const juce::String &logname);
        std::unique_ptr<juce::FileOutputStream>     _stream;

        void setup(const juce::String &logFileName);
};

#endif
