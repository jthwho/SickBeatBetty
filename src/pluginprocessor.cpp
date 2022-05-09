
#include "pluginprocessor.h"
#include "plugineditor.h"
#include "buildinfo.h"
#include "applogger.h"

#define APP_NAME "SickBeatBetty"
static const juce::Identifier ParamStateIdentifier("ParamState");

static int registerPluginProcessor(PluginProcessor *p) {
    juce::ignoreUnused(p);
    static int _pluginProcID = 0;
    AppLogger::instance(); // Call this to make sure the app logger instance has been created.
    return _pluginProcID++;
}

PluginProcessor::PluginProcessor() : 
    AudioProcessor(
        BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo())
    ),
    _index(registerPluginProcessor(this)),
    _beatGen(beatGenCount),
    _params(*this, nullptr, ParamStateIdentifier, createParameterLayout()),
    _programManager(APP_NAME, _params, nullptr)
{
    juce::Logger::writeToLog(juce::String("Starting up PluginProcessor ") + juce::String(_index) + " for " + getWrapperTypeDescription(wrapperType));
    for(int i = 0; i < _beatGen.size(); i++) _beatGen[i].attachParams(_params);
    _bpm = _params.getRawParameterValue("bpm");
    _programManager.addListener(this);
    addProgramChangeActionListener(&_programManager);
}

PluginProcessor::~PluginProcessor() {
    _programManager.removeListener(this);
    removeProgramChangeActionListener(&_programManager);
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout() const {
    juce::AudioProcessorValueTreeState::ParameterLayout ret;
    // If we're standalone, the BPM needs to come from a parameter.  If not,
    // we get it from the playhead
    if(wrapperType == wrapperType_Standalone) {
        ret.add(std::make_unique<juce::AudioParameterFloat>(
            "bpm", 
            "BPM",
            1.0f, 999.0f, 120.0f
        ));
    }
    for(int i = 0; i < _beatGen.size(); i++) ret.add(_beatGen[i].createParameterLayout());
    return ret;
}

const juce::String PluginProcessor::getName() const {
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const {
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const {
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const {
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int PluginProcessor::getNumPrograms() {
    // FIXME: This is a hack to have to keep from updating the program number count.
    return 100;
    //return _programManager.programCount();
}

int PluginProcessor::getCurrentProgram() {
    return _programManager.currentProgram();
}

void PluginProcessor::setCurrentProgram(int index) {
    _hostProgram = index;
    juce::Logger::writeToLog("Host Program Change " + juce::String(index));
    juce::String msg = "ProgramChange " + juce::String(index);
    _programChangeActionBroadcaster.sendActionMessage(msg);
    return;
}

const juce::String PluginProcessor::getProgramName(int index) {
    return juce::String(index);
}

void PluginProcessor::changeProgramName(int index, const juce::String& newName) {
    _programManager.renameProgram(index, newName);
    return;
}

void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::ignoreUnused(samplesPerBlock);
    _sampleRate = sampleRate;
    return;
}

void PluginProcessor::releaseResources() {
    return;
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
    return !layouts.getMainInputChannelSet().isDisabled();
}

void PluginProcessor::processBlock(juce::AudioBuffer<float> &audio, juce::MidiBuffer &midi) {
    juce::AudioPlayHead::CurrentPositionInfo pos;
    juce::AudioPlayHead *ph = getPlayHead();
    double bpm = 120.0;
    bool transportRunning = true;

    if(ph != nullptr) {
        // If we've got a playhead, we're running as a plugin
        ph->getCurrentPosition(pos);
        bpm = pos.bpm;
        transportRunning = pos.isPlaying;
        _now = pos.ppqPosition;
    } else if(_bpm != nullptr) {
        // If we've got a _bpm parameter, we're running standalone
        bpm = *_bpm;
        transportRunning = true;
    }

    /*
    int programChange = -1;
    for(auto item : midi) {
        auto msg = item.getMessage();
        if(msg.isController() && msg.getControllerNumber() == 127) {
            programChange = msg.getControllerValue();
        }
    }
    */
 
    if(transportRunning != _transportRunning) {
        juce::Logger::writeToLog(juce::String("Transport ") + (transportRunning ? "Running" : "Stopped"));
        _transportRunning = transportRunning;
    }

    BeatGen::GenerateState genState;
    double qnPerBar = 4.0; // FIXME: We should probably read the time signature here?
    double qnPerSample = bpm / 60.0 / _sampleRate; // Quarter notes per sample (going to be a small fraction of a quarter note)
    double samplesPerBlock = (double)audio.getNumSamples();
    double qnPerBlock = qnPerSample * samplesPerBlock;
    genState.enabled = transportRunning;
    genState.start = _now / qnPerBar; 
    genState.end = (_now + qnPerBlock) / qnPerBar;
    genState.stepSize = qnPerSample / qnPerBar;
    
    //printf("%lf bpm, %d samples, %lf qnPerSample, %lf start, %lf end\n",
    //    bpm, audio.getNumSamples(), qnPerSample, genState.start, genState.end);
    // FIXME: Move this loop into the BeatGenGroup object
    bool noSolo = !_beatGen.isSoloed(); 
    for(int i = 0; i < _beatGen.size(); i++) {
        BeatGen &gen = _beatGen[i];
        if(noSolo || gen.isSolo()) {
            _beatGen[i].generate(genState, midi);
        }
    }

    /*
    if(programChange != -1) {
        juce::Logger::writeToLog("Program Change " + juce::String(programChange));
        juce::String msg = "ProgramChange " + juce::String(programChange);
        _programChangeActionBroadcaster.sendActionMessage(msg);
    }
    */

    _now += qnPerBlock;
    return;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new PluginProcessor();
}

juce::AudioProcessorEditor * PluginProcessor::createEditor() {
    // Uncomment and comment the other one to test our custom GUI.
    return new PluginEditor(*this, _params);
    //return new juce::GenericAudioProcessorEditor(*this);
}

bool PluginProcessor::hasEditor() const {
    return true;
}

void PluginProcessor::getStateInformation(juce::MemoryBlock &destData) {
    StateXML xml = _programManager.getStateXML();
    copyXmlToBinary(*xml, destData);
    juce::Logger::writeToLog("Saved state");
    return;
}

void PluginProcessor::setStateInformation(const void *data, int sizeInBytes) {
    StateXML xml(getXmlFromBinary(data, sizeInBytes));
    if(xml.get() == nullptr) {
        juce::Logger::writeToLog("Failed to parse state XML");
        return;
    }
    _programManager.setStateFromXML(xml);
    return;
}

void PluginProcessor::programManagerProgramChanged(int value) {
    int max = _programManager.programCount();
    int hostValue = _hostProgram;
    if(hostValue < 0) hostValue = 0;
    if(hostValue >= max) hostValue = max - 1;
    if(hostValue == value) return; // The host and the program manager agree, no need to update the host.
    _hostProgram = value;
    ChangeDetails details;
    details.programChanged = true;
    updateHostDisplay(details);
    juce::Logger::writeToLog("Update host on program change " + juce::String(value));
    return;
}

void PluginProcessor::programManagerListChanged() {
    //ChangeDetails details;
    //details.parameterInfoChanged = true;
    //updateHostDisplay(details);
    //juce::Logger::writeToLog("Update host on program list change");
    return;
}
