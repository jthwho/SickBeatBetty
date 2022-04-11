#include "PluginProcessor.h"
#include "PluginEditor.h"

PluginProcessor::PluginProcessor() : 
    AudioProcessor(
        BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo())
    ),
    _beatGen(),
    _params(*this, nullptr, juce::Identifier("params"), createParameterLayout())
{
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
    for(auto &i : _beatGen) i.reset();
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
        transportRunning = pos.isPlaying || pos.isRecording;
        _now = pos.timeInSeconds;
    } else if(_bpm != nullptr) {
        // If we've got a _bpm parameter, we're running standalone
        bpm = *_bpm;
        transportRunning = true;
    }

    // Check for a transport state change.
    if(transportRunning != _transportRunning) {
        printf("Transport %s\n", transportRunning ? "Running" : "Stopped");
        if(transportRunning) {
            //if(ph != nullptr) {
            //    _now = pos.timeInSeconds;
            //    printf("Reset position to %lf from transport\n", _now);
            //}
            for(auto &i : _beatGen) i.reset();
        } else {
            _now = 0.0;
        }
        _transportRunning = transportRunning;
    }

    BeatGen::GenerateState genState;
    double steps = (double)audio.getNumSamples();
    double genLen = steps / _sampleRate;
    double phaseLen = 60.0 / bpm * 4.0;
    genState.enabled = transportRunning;
    genState.stepSize = 1.0 / _sampleRate;
    genState.start = _now / phaseLen;
    genState.end = (_now + genLen) / phaseLen;
    
    if(transportRunning) {
        for(auto &i : _beatGen) i.generate(genState, midi);
    }
    _now += genLen;
    return;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new PluginProcessor();
}

juce::AudioProcessorEditor * PluginProcessor::createEditor() {
    //return new PluginEditor(*this, _params);
    return new juce::GenericAudioProcessorEditor(*this);
}

bool PluginProcessor::hasEditor() const {
    return true;
}

void PluginProcessor::getStateInformation(juce::MemoryBlock &destData) {
    auto state = _params.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
    return;
}

void PluginProcessor::setStateInformation(const void *data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary (data, sizeInBytes));
    if(xmlState.get() != nullptr) {
        if(xmlState->hasTagName(_params.state.getType())) {
            _params.replaceState (juce::ValueTree::fromXml(*xmlState));
        }
    }
    return;
}
