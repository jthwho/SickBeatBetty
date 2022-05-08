#include "programmanager.h"
#include "buildinfo.h"

#define STATE_NAME      "HowardLogicState"
#define STATE_VERSION   1

static const juce::Identifier NameIdentifier("name");
static const juce::Identifier AppNameIdentifier("appName");
static const juce::Identifier AppStateIdentifier("AppState");
static const juce::Identifier ProgramStateIdentifier("ProgramState");

ProgramManager::ProgramManager(const juce::String &appName, juce::AudioProcessorValueTreeState &vts, juce::UndoManager *undo) :
    _undo(undo),
    _vts(vts),
    _appState(AppStateIdentifier),
    _programState(ProgramStateIdentifier)
{
    _appState.setProperty(AppNameIdentifier, appName, nullptr);
    _programState.setProperty(NameIdentifier, "Program 1", nullptr);
    // We should always have one program.
    _programStateArray.add(_programState);
    _vtsStateArray.add(_vts.copyState());

    addProgram("Test Program 2");
    addProgram("Test Program 3");
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
    if(index == _currentProgram || !indexIsValid(index)) return;
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
    if(!indexIsValid(index)) return;
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
    juce::ignoreUnused(indexToCopy);
    // TODO
    return;
}

void ProgramManager::addProgram(const juce::String &name) {
    juce::String actualName = name.isEmpty() ? 
        juce::String("Program ") + juce::String(programCount() + 1) :
        name;
    juce::ValueTree programState = _programState.createCopy();
    programState.setProperty(NameIdentifier, actualName, nullptr);
    _programStateArray.add(programState);
    _vtsStateArray.add(_vts.copyState());
    return;
}

void ProgramManager::deleteProgram(int indexToDelete) {
    juce::ignoreUnused(indexToDelete);
    // TODO
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

    juce::String xmlstr = ret->toString();
    printf(xmlstr.toStdString().c_str());
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
