#include "programmanager.h"
#include "buildinfo.h"

#define STATE_NAME      "HowardLogicState"
#define STATE_VERSION   1

static const juce::Identifier NodeIDIdentifier("NodeID");
static const juce::Identifier NameIdentifier("name");
static const juce::Identifier AppNameIdentifier("appName");
static const juce::Identifier AppStateIdentifier("AppState");
static const juce::Identifier ProgramStateIdentifier("ProgramState");

juce::File ProgramManager::userStateStoragePath() {
    auto ret = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("SickBeatBetty");
    if(!ret.isDirectory()) ret.createDirectory();
    return ret;
}

ProgramManager::PresetInfoArray ProgramManager::getPresetsInFolder(const juce::File &path) {
    PresetInfoArray ret;
    if(!path.isDirectory()) return ret;
    printf("Scanning %s for presets...\n", path.getFullPathName().toStdString().c_str());
    auto files = path.findChildFiles(
        juce::File::findFiles | juce::File::ignoreHiddenFiles,
        false, "*.preset");
    for(int i = 0; i < files.size(); i++) {
        PresetInfo info;
        info.index = i;
        info.path = files[i].getFullPathName();
        ret.add(info);
    }
    return ret;
}

ProgramManager::ProgramManager(const juce::String &appName, juce::AudioProcessorValueTreeState &vts, juce::UndoManager *undo) :
    _undo(undo),
    _vts(vts),
    _appState(AppStateIdentifier),
    _programState(ProgramStateIdentifier)
{
    _appState.setProperty(AppNameIdentifier, appName, nullptr);
    _appState.setProperty(NodeIDIdentifier, juce::Uuid().toString(), nullptr);
    _programState.setProperty(NameIdentifier, "Default Program", nullptr);
    _programState.setProperty(NodeIDIdentifier, juce::Uuid().toString(), nullptr);
    // We should always have one program.
    _programStateArray.add(_programState);
    _vtsStateArray.add(_vts.copyState());

}

ProgramManager::~ProgramManager()
{

}

juce::ValueTree &ProgramManager::programStateForIndex(int index) {
    return index == _currentProgram ? _programState : _programStateArray.getReference(index);
}

const juce::ValueTree &ProgramManager::programStateForIndex(int index) const {
    return index == _currentProgram ? _programState : _programStateArray.getReference(index);
}

void ProgramManager::changeProgram(int index) {
    if(index == _currentProgram || !indexIsValid(index)) {
        juce::Logger::writeToLog(juce::String::formatted(
            "Failed to change program %d, current %d, size %d",
            index, _currentProgram, programCount()));
        return;
    }
    juce::Logger::writeToLog(juce::String::formatted("Change program %d", index));
    syncToArray(); // Write the current state of things into the program array.
    _currentProgram = index;
    syncFromArray(); // Load the newly selected index from the program array.
    _listenerList.call(
        [this](Listener &l) { l.programManagerProgramChanged(_currentProgram); }
    );
    return;
}

juce::String ProgramManager::programName(int index) const {
    if(!indexIsValid(index)) return juce::String();
    return programStateForIndex(index).getProperty(NameIdentifier).toString();
}

void ProgramManager::renameProgram(int index, const juce::String &name) {
    if(!indexIsValid(index)) {
        juce::Logger::writeToLog(juce::String::formatted("Failed to rename program %d", index));
        return;
    }
    juce::Logger::writeToLog(juce::String::formatted("Rename program %d: ", index) + name);
    juce::ValueTree &state = programStateForIndex(index);
    if(state.isValid()) {
        state.setProperty(NameIdentifier, name, _undo);
        if(index == _currentProgram) {
            _listenerList.call(
                [](Listener &l) { l.programManagerCurrentProgramNamedChanged(); }
            );
        }
    }
    return;
}

void ProgramManager::duplicateProgram(int indexToCopy) {
    if(!indexIsValid(indexToCopy)) return;
    juce::ValueTree programState, vtsState;
    if(indexToCopy == _currentProgram) {
        programState = _programState.createCopy();
        vtsState = _vts.copyState();
    } else {
        programState = _programStateArray[indexToCopy].createCopy();
        vtsState = _vtsStateArray[indexToCopy].createCopy();
    }
    juce::String name = programState.getProperty(NameIdentifier).toString();
    name += " Copy";
    programState.setProperty(NameIdentifier, name, nullptr);
    programState.setProperty(NodeIDIdentifier, juce::Uuid().toString(), nullptr);
    _programStateArray.add(programState);
    _vtsStateArray.add(vtsState);
    _listenerList.call(
        [](Listener &l) { l.programManagerListChanged(); }
    );
    return;
}

void ProgramManager::deleteProgram(int indexToDelete) {
    if(programCount() < 2 || !indexIsValid(indexToDelete)) {
        juce::Logger::writeToLog(juce::String::formatted("Failed to delete %d, %d in list", indexToDelete, programCount()));
        return; // We never allow delete of the last program.
    }
    juce::Logger::writeToLog(juce::String::formatted("Delete program %d", indexToDelete));
    // If the index to delete is the current program, we've got to first move off it,
    if(indexToDelete == _currentProgram) {
        int nextProgram = _currentProgram - 1;
        if(nextProgram == -1) nextProgram = _currentProgram + 1;
        changeProgram(nextProgram);
    }
    // Now we're free to remove the program from the index.
    _programStateArray.remove(indexToDelete);
    _vtsStateArray.remove(indexToDelete);
    // Now, make sure we update the current program index if it was below
    // The indexToDelete
    if(indexToDelete < _currentProgram) _currentProgram--;
    // And let everyone know.
    _listenerList.call([](Listener &l) {
        l.programManagerListChanged();
    });
    return;
}

void ProgramManager::overwriteProgram(int indexToCopy, int indexToOverwrite) {
    juce::ignoreUnused(indexToCopy, indexToOverwrite);
    // TODO
    return;
}

void ProgramManager::syncToArray() {
    _programStateArray.set(_currentProgram, _programState.createCopy());
    _vtsStateArray.set(_currentProgram, _vts.copyState());
    return;
}

void ProgramManager::syncFromArray() {
    _programState.copyPropertiesAndChildrenFrom(_programStateArray[_currentProgram], nullptr);
    _vts.replaceState(_vtsStateArray[_currentProgram]);
    return;
}

ProgramManager::StateXML ProgramManager::getStateXML() {
    syncToArray();
    const BuildInfo *buildInfo = getBuildInfo();
    StateXML ret = std::make_unique<juce::XmlElement>(STATE_NAME);
    ret->setAttribute("stateVersion", STATE_VERSION);
    ret->setAttribute("currentProgram", _currentProgram);

    // Add the app state information
    ret->addChildElement(_appState.createXml().release());

    // Add the build information (not used by the loader, just useful to have for debug)
    auto buildInfoNode = ret->createNewChildElement("BuildInfo");
    buildInfoNode->createNewChildElement("version")->addTextElement(buildInfo->version);
    buildInfoNode->createNewChildElement("repoident")->addTextElement(buildInfo->repoident);
    buildInfoNode->createNewChildElement("date")->addTextElement(buildInfo->date);
    buildInfoNode->createNewChildElement("time")->addTextElement(buildInfo->time);

    // Add the saver information (not used by the loader, just useful to have for debug)
    auto saverInfoNode = ret->createNewChildElement("SaverInfo");
    saverInfoNode->createNewChildElement("timestamp")->addTextElement(juce::Time::getCurrentTime().toISO8601(true));
    saverInfoNode->createNewChildElement("os")->addTextElement(juce::SystemStats::getOperatingSystemName());
    saverInfoNode->createNewChildElement("juceVersion")->addTextElement(juce::SystemStats::getJUCEVersion());

    // Add the program states array
    auto programStatesNode = ret->createNewChildElement("ProgramStates");
    programStatesNode->setAttribute("count", _programStateArray.size());
    for(int i = 0; i < _programStateArray.size(); i++) {
        auto item = _programStateArray.getReference(i).createXml();
        item->setAttribute("index", i);
        programStatesNode->addChildElement(item.release());
    }

    // Add the param states array
    auto paramStatesNode = ret->createNewChildElement("ParamStates");
    paramStatesNode->setAttribute("count", _vtsStateArray.size());
    for(int i = 0; i < _vtsStateArray.size(); i++) {
        auto item = _vtsStateArray.getReference(i).createXml();
        item->setAttribute("index", i);
        paramStatesNode->addChildElement(item.release());
    }
    return ret;
}

static bool loadValueTreeArrayXML(juce::Array<juce::ValueTree> &array, juce::XmlElement &xml) {
    int count = xml.getIntAttribute("count", -1);
    if(count < 1) {
        juce::Logger::writeToLog("State XML " + xml.getTagName() + " has an invalid count: " + juce::String(count));
        return false;
    }
    for(auto *child : xml.getChildIterator()) {
        auto tree = juce::ValueTree::fromXml(*child);
        if(!tree.isValid()) {
            juce::Logger::writeToLog("State XML " + xml.getTagName() + " child failed to parse: " + juce::String(array.size()));
            return false;
        }
        array.add(tree);
    }
    if(count != array.size()) {
        juce::Logger::writeToLog("State XML " + xml.getTagName() + 
            " child count mismatch.  Expected " + juce::String(count) + 
            " got " + juce::String(array.size()));
        return false;
    }
    return true;
}

bool ProgramManager::setStateFromXMLv1(const StateXML &xml) {
    juce::ValueTree appState;
    juce::Array<juce::ValueTree> programStateArray;
    juce::Array<juce::ValueTree> vtsStateArray;
    int currentProgram = xml->getIntAttribute("currentProgram", 0);

    auto appStateNode = xml->getChildByName(AppStateIdentifier);
    if(appStateNode == nullptr) {
        juce::Logger::writeToLog("State XML had no AppState node");
        return false;
    }
    appState = juce::ValueTree::fromXml(*appStateNode);
    if(!appState.isValid()) {
        juce::Logger::writeToLog("State XML failed to parse AppState node");
        return false;
    }
    if(appState.getProperty(AppNameIdentifier) != _appState.getProperty(AppNameIdentifier)) {
        juce::Logger::writeToLog("State XML appName is wrong, expected '" + 
            _appState.getProperty(AppNameIdentifier).toString() + 
            "' got '" +
            appState.getProperty(AppNameIdentifier).toString());
        return false;
    }

    auto programStatesNode = xml->getChildByName("ProgramStates");
    if(programStatesNode == nullptr) {
        juce::Logger::writeToLog("State XML had no ProgramStates node");
        return false;
    }
    if(!loadValueTreeArrayXML(programStateArray, *programStatesNode)) return false;

    auto paramStatesNode = xml->getChildByName("ParamStates");
    if(paramStatesNode == nullptr) {
        juce::Logger::writeToLog("State XML had no ParamStates node");
        return false;
    }
    if(!loadValueTreeArrayXML(vtsStateArray, *paramStatesNode)) return false;

    // Some final checks to make sure the state is well formed.
    if(vtsStateArray.size() != programStateArray.size()) {
        juce::Logger::writeToLog("State XML param and program arrays differ " +
            juce::String(vtsStateArray.size()) + " vs " + juce::String(programStateArray.size()));
        return false;
    }

    if(vtsStateArray.size() < 1) {
        juce::Logger::writeToLog("State XML doesn't have at least one entry");
        return false;
    }

    if(currentProgram < 0 || currentProgram >= vtsStateArray.size()) {
        juce::Logger::writeToLog("State XML currentProgram is out of bounds " + juce::String(currentProgram));
        currentProgram = 0; // Failback to a safe value that we know exists.
    }

    // Nothing left to do but swap the state out.
    _currentProgram = currentProgram;
    _programStateArray = programStateArray;
    _vtsStateArray = vtsStateArray;
    _appState = appState;
    syncFromArray();
    return true;
}

bool ProgramManager::setStateFromXML(const StateXML &xml) {
    if(xml->getTagName() != STATE_NAME) {
        juce::Logger::writeToLog(juce::String("State XML tag name is incorrect. Expected ") + STATE_NAME + ", got " + xml->getTagName());
        return false;
    }

    int stateVersion = xml->getIntAttribute("stateVersion", -1);
    bool ret = false;
    switch(stateVersion) {
        case 1: ret = setStateFromXMLv1(xml); break;
        default:
            juce::Logger::writeToLog(juce::String::formatted("State XML version %d isn't supported", stateVersion));
            ret = false;
            break;
    }
    return ret;
}

void ProgramManager::actionListenerCallback(const juce::String &message) {
    juce::StringArray tokens = juce::StringArray::fromTokens(message, false);
    if(tokens.size() < 2) return;
    if(tokens[0] == "ProgramChange") {
        int val = tokens[1].getIntValue();
        if(val < 0) val = 0;
        if(val >= _programStateArray.size()) val = _programStateArray.size() - 1;
        changeProgram(val);
    }
    return;
}
