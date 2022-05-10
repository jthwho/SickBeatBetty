#ifndef _PROGRAMMANAGER_H_
#define _PROGRAMMANAGER_H_
#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include <juce_audio_processors/juce_audio_processors.h>

class ProgramManager : 
    public juce::ActionListener
{
    public:
        typedef std::unique_ptr<juce::XmlElement> StateXML;

        class Listener {
            public:
                virtual ~Listener() { };
                virtual void programManagerProgramChanged(int value) { 
                    juce::ignoreUnused(value);
                };
                virtual void programManagerCurrentProgramNamedChanged() { };
                virtual void programManagerListChanged() { };
        };

        static juce::File userStateStoragePath();

        ProgramManager(const juce::String &appName, juce::AudioProcessorValueTreeState &vts, juce::UndoManager *undo);
        ~ProgramManager();

        // Acces to the app state tree.  This is a single tree that persists
        // across the entire application.
        juce::ValueTree &appState();
        const juce::ValueTree &appState() const;

        // Access to the program state.  The values will be swapped out on program change
        juce::ValueTree &programState();
        const juce::ValueTree &programState() const;

        bool indexIsValid(int index) const;
        int currentProgram() const;
        int programCount() const;
        void changeProgram(int index);
        void renameProgram(int index, const juce::String &name);
        void duplicateProgram(int indexToCopy);
        void deleteProgram(int indexToDelete);
        void overwriteProgram(int indexToCopy, int indexToOverwrite);
        juce::String programName(int index) const;

        StateXML getStateXML();
        bool setStateFromXML(const StateXML &xml);

        void addListener(Listener *listener);
        void removeListener(Listener *listener);

    private:
        int                                 _currentProgram = 0;
        juce::UndoManager                   *_undo = nullptr;
        juce::AudioProcessorValueTreeState  &_vts;
        juce::ValueTree                     _programState;
        juce::ValueTree                     _appState;
        juce::Array<juce::ValueTree>        _programStateArray;
        juce::Array<juce::ValueTree>        _vtsStateArray;
        juce::ListenerList<Listener>        _listenerList; 

        bool setStateFromXMLv1(const StateXML &xml);

        juce::ValueTree &programStateForIndex(int index);
        const juce::ValueTree &programStateForIndex(int index) const;

        void syncToArray();
        void syncFromArray();

        void actionListenerCallback(const juce::String &message);
};

inline juce::ValueTree &ProgramManager::appState() {
    return _appState;
}

inline const juce::ValueTree &ProgramManager::appState() const {
    return _appState;
}

inline juce::ValueTree &ProgramManager::programState() {
    return _programState;
}

inline const juce::ValueTree &ProgramManager::programState() const {
    return _programState;
}

inline bool ProgramManager::indexIsValid(int index) const {
    return index >= 0 && index < _programStateArray.size();
}

inline int ProgramManager::currentProgram() const {
    return _currentProgram;
}

inline int ProgramManager::programCount() const {
    return _programStateArray.size();
}

inline void ProgramManager::addListener(Listener *listener) {
    _listenerList.add(listener);
    return;
}

inline void ProgramManager::removeListener(Listener *listener) {
    _listenerList.remove(listener);
    return;
}

#endif

