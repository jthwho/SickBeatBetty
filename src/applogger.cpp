#include <iostream>
#include "applogger.h"

#define LOGFILE_NAME    "sickbeatbetty.log"

AppLogger &AppLogger::instance() {
    static AppLogger _logger(LOGFILE_NAME);
    return _logger;
}

AppLogger::AppLogger(const juce::String &fn) {
    setup(fn);
}

AppLogger::~AppLogger() {
    _stream->flush();
    if(this == juce::Logger::getCurrentLogger()) juce::Logger::setCurrentLogger(nullptr);
}

void AppLogger::setup(const juce::String &filename) {
    juce::File logdir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    juce::File file = logdir.getChildFile(filename);
    juce::Result result = file.create();
    if(result.failed()) {
        std::cout << "Failed to create " << file.getFullPathName() << ": " << result.getErrorMessage() << std::endl;
        return;
    }
    _stream = std::make_unique<juce::FileOutputStream>(file);
    if(_stream->failedToOpen()) {
        std::cout << "Failed to open " << file.getFullPathName() << std::endl;
        return;
    }
    // Set thyself as the system logger.
    juce::Logger::setCurrentLogger(this);
    std::cout << "Logging to: " << file.getFullPathName() << std::endl;
    return;
}

void AppLogger::logMessage(const juce::String &msg) {
    juce::Time now = juce::Time::getCurrentTime();
    juce::String nowString = juce::String::formatted(
        "%04d-%02d-%02d %02d:%02d:%02d.%03d",
        now.getYear(), now.getMonth() + 1, now.getDayOfMonth(),
        now.getHours(), now.getMinutes(), now.getSeconds(), now.getMilliseconds()
    );
    *_stream << nowString << ": " << msg << "\n";
    _stream->flush();
    return;
}
