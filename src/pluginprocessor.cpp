
#include "pluginprocessor.h"
#include "plugineditor.h"
#include "buildinfo.h"

#define STATE_VERSION       1
#define STATE_NAME          "SickBeatBettyState"

PluginProcessor::PluginProcessor() : 
    AudioProcessor(
        BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo())
    ),
    _logger("sickbeatbetty.log"),
    _beatGen(),
    _params(*this, nullptr, juce::Identifier("params"), createParameterLayout()),
    _props("props")
{
    juce::Logger::writeToLog(juce::String("Starting up PluginProcessor for ") + getWrapperTypeDescription(wrapperType));
    for(auto &i : _beatGen) i.attachParams(_params);
    _bpm = _params.getRawParameterValue("bpm");
}

PluginProcessor::~PluginProcessor() {

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
    for(auto &i : _beatGen) ret.add(i.createParameterLayout());
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
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram() {
    return 0;
}

void PluginProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
    return;
}

const juce::String PluginProcessor::getProgramName(int index) {
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName(int index, const juce::String& newName) {
    juce::ignoreUnused(index, newName);
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
    for(auto &i : _beatGen) i.generate(genState, midi);
    
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
    const BuildInfo *buildInfo = getBuildInfo();

    juce::XmlElement root(STATE_NAME);
    root.setAttribute("version", STATE_VERSION);
    root.addChildElement(_params.copyState().createXml().release());
    root.addChildElement(_props.createXml().release());

    auto buildInfoNode = root.createNewChildElement("BuildInfo");
    buildInfoNode->createNewChildElement("version")->addTextElement(buildInfo->version);
    buildInfoNode->createNewChildElement("repoident")->addTextElement(buildInfo->repoident);
    buildInfoNode->createNewChildElement("date")->addTextElement(buildInfo->date);
    buildInfoNode->createNewChildElement("time")->addTextElement(buildInfo->time);

    auto saverInfoNode = root.createNewChildElement("SaverInfo");
    saverInfoNode->createNewChildElement("timestamp")->addTextElement(juce::Time::getCurrentTime().toISO8601(true));
    saverInfoNode->createNewChildElement("os")->addTextElement(juce::SystemStats::getOperatingSystemName());
    saverInfoNode->createNewChildElement("juceVersion")->addTextElement(juce::SystemStats::getJUCEVersion());
    saverInfoNode->createNewChildElement("wrapperType")->addTextElement(juce::String(wrapperType));
    saverInfoNode->createNewChildElement("wrapperName")->addTextElement(getWrapperTypeDescription(wrapperType));
    //printf("GET STATE:\n%s\n", root.toString().toStdString().c_str());
    copyXmlToBinary(root, destData);
    juce::Logger::writeToLog("Saved state");
    return;
}

void PluginProcessor::setStateInformation(const void *data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary (data, sizeInBytes));
    if(xml.get() == nullptr) {
        juce::Logger::writeToLog("Failed to parse state XML");
        return;
    }
    if(xml->getTagName() != STATE_NAME) {
        juce::Logger::writeToLog(juce::String("State XML tag name is incorrect. Expected ") + STATE_NAME + ", got " + xml->getTagName());
        return;
    }
    
    // This will allow us to support multiple state versions in the future if we need to.
    int stateVersion = xml->getIntAttribute("version", -1);
    if(stateVersion == STATE_VERSION) {
        auto paramsXml = xml->getChildByName(_params.state.getType());
        if(paramsXml == nullptr) {
            juce::Logger::writeToLog("Failed to get params node from state XML");
            return;
        }

        auto propsXml = xml->getChildByName(_props.getType());
        if(propsXml == nullptr) {
            juce::Logger::writeToLog("Failed to get props node from state XML");
            return;
        }

        auto params = juce::ValueTree::fromXml(*paramsXml);
        if(!params.isValid()) {
            juce::Logger::writeToLog("Failed to parse params from state XML");
            return;
        }

        auto props = juce::ValueTree::fromXml(*propsXml);
        if(!props.isValid()) {
            juce::Logger::writeToLog("Failed to parse props from state XML");
            return;
        }
        _params.replaceState(params);
        _props.copyPropertiesAndChildrenFrom(props, nullptr);
        juce::Logger::writeToLog("Loaded state");

    } else {
        juce::Logger::writeToLog(juce::String::formatted("State XML version %d isn't supported", stateVersion));
    }
    return;
}
