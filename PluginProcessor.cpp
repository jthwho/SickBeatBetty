#include "PluginProcessor.h"
#include "PluginEditor.h"

PluginProcessor::PluginProcessor() : 
    AudioProcessor(
        BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo())
    ),
    _params(*this, nullptr, juce::Identifier("params"), juce::AudioProcessorValueTreeState::ParameterLayout())
{
    _beatGen.attachParameters(_params);   
}

PluginProcessor::~PluginProcessor() {

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
    _beatGen.reset(sampleRate);
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
    if(ph) {
        ph->getCurrentPosition(pos);

        bool transportRunning = pos.isPlaying || pos.isRecording;
        if(transportRunning != _transportRunning) {
            printf("Transport %s\n", transportRunning ? "Running" : "Stopped");
            if(transportRunning) {
                _beatGen.reset();
            }
            _transportRunning = transportRunning;
        }
    }

    //jassert(audio.getNumChannels() == 0); // We're a MIDI plugin, so we shouldn't have any audio.
    auto samples = audio.getNumSamples(); // But, we do get a sample count to we can keep track of time.
    
    midi.clear();
    _beatGen.processBlock(audio, midi);

    return;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new PluginProcessor();
}

juce::AudioProcessorEditor * PluginProcessor::createEditor() {
    //return new PluginEditor(*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

bool PluginProcessor::hasEditor() const {
    return true;
}

void PluginProcessor::getStateInformation(juce::MemoryBlock &destData) {
    return;
}

void PluginProcessor::setStateInformation(const void *data, int sizeInBytes) {
    return;
}
