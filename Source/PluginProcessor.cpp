
#include "PluginProcessor.h"
#include "PluginEditor.h"

static const int numberOfVoices = 10;

//==============================================================================
MySynthesiserVoice::MySynthesiserVoice() {
    oscillator.setup(getSampleRate());
    envelope.setSampleRate(getSampleRate());

    
}

bool MySynthesiserVoice::canPlaySound(SynthesiserSound *sound) {
    return dynamic_cast<MySynthesiserSound *> (sound) != nullptr;
}

void MySynthesiserVoice::startNote(int midiNoteNumber, float velocity,
                                   SynthesiserSound *, int currentPitchWheelPosition) {
    oscillator.reset();
    level = velocity * 0.15;
    envelope.noteOn();
    auto m_freq = (MidiMessage::getMidiNoteInHertz (midiNoteNumber));
    oscillator.setFreq(m_freq);
    oscillator.set_note_velocity(velocity);

    
}

void MySynthesiserVoice::stopNote(float /*velocity*/, bool allowTailOff) {
    envelope.noteOff();
    //oscillator.reset();
    
}


void MySynthesiserVoice::pitchWheelMoved(int value){
    oscillator.setPitchBend(value);

}





void MySynthesiserVoice::renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples) {
    while (--numSamples >= 0) {
        
        
        //oscillator.setPitchBend(MidiMessage::getPitchWheelValue());
        auto envAmp = envelope.getNextSample();
        auto thisSample = oscillator.process();
        auto currentSample = thisSample * level * envAmp;
        //vadimFilter.setCutoffFrequency(500);
        //vadimFilter.setCutoffFrequency(2000 + currentSample*1000);
        //currentSample = vadimFilter.processSample(1, currentSample);

        for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
            outputBuffer.addSample(i, startSample, currentSample);

        ++startSample;
        
    

        if (envAmp <= 0.0) {
            clearCurrentNote();
            break;
        }
    }
}

void MySynthesiserVoice::setEnvelopeParameters(ADSR::Parameters params) {
    envelope.setParameters(params);
}

void MySynthesiserVoice::setWavetypeParameter(int type) {
    oscillator.setWavetype(type);
}

void MySynthesiserVoice::setSharpParameter(float sharp) {
    oscillator.setSharp(sharp);
}

void MySynthesiserVoice::setModParameter(float mod) {
    oscillator.setMod(mod);
}

//==============================================================================
void MySynthesiser::setEnvelopeParameters(ADSR::Parameters params) {
    for (int i = 0; i < getNumVoices(); i++)
       dynamic_cast<MySynthesiserVoice *> (getVoice(i))->setEnvelopeParameters(params);
}

void MySynthesiser::setWavetypeParameter(int type) {
    for (int i = 0; i < getNumVoices(); i++)
       dynamic_cast<MySynthesiserVoice *> (getVoice(i))->setWavetypeParameter(type);
}

void MySynthesiser::setSharpParameter(float sharp) {
    for (int i = 0; i < getNumVoices(); i++)
       dynamic_cast<MySynthesiserVoice *> (getVoice(i))->setSharpParameter(sharp);
}

void MySynthesiser::setModParameter(float mod) {
    for (int i = 0; i < getNumVoices(); i++)
       dynamic_cast<MySynthesiserVoice *> (getVoice(i))->setModParameter(mod);
}

//==============================================================================
static String secondSliderValueToText(float value) {
    return String(value, 3) + String(" sec");
}

static float secondSliderTextToValue(const String& text) {
    return text.getFloatValue();
}

static String sharpSliderValueToText(float value) {
    return String(value, 4) + String(" x");
}

static float sharpSliderTextToValue(const String& text) {
    return text.getFloatValue();
}

static float modSliderTextToValue(const String& text) {
    return text.getFloatValue();
}

static String gainSliderValueToText(float value) {
    float val = 20.0f * log10f(jmax(0.001f, value));
    return String(val, 2) + String(" dB");
}

static float gainSliderTextToValue(const String& text) {
    float val = jlimit(-60.0f, 18.0f, text.getFloatValue());
    return powf(10.0f, val * 0.05f);
}

AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
    using Parameter = AudioProcessorValueTreeState::Parameter;

    std::vector<std::unique_ptr<Parameter>> parameters;

    parameters.push_back(std::make_unique<Parameter>(String("attack"), String("Attack"), String(),
                                                     NormalisableRange<float>(0.001f, 1.f, 0.001f, 0.5f),
                                                     0.005f, secondSliderValueToText, secondSliderTextToValue));

    parameters.push_back(std::make_unique<Parameter>(String("decay"), String("Decay"), String(),
                                                     NormalisableRange<float>(0.001f, 1.f, 0.001f, 0.5f),
                                                     0.2f, secondSliderValueToText, secondSliderTextToValue));

    parameters.push_back(std::make_unique<Parameter>(String("sustain"), String("Sustain"), String(),
                                                     NormalisableRange<float>(0.001f, 1.f, 0.001f, 0.3f),
                                                     0.708f, gainSliderValueToText, gainSliderTextToValue));

    parameters.push_back(std::make_unique<Parameter>(String("release"), String("Release"), String(),
                                                     NormalisableRange<float>(0.001f, 1.f, 0.001f, 0.5f),
                                                     0.25f, secondSliderValueToText, secondSliderTextToValue));

    parameters.push_back(std::make_unique<Parameter>(String("type"), String("Type"), String(),
                                                     NormalisableRange<float>(0.0f, 8.0f, 1.f, 1.0f),
                                                     0.0f, nullptr, nullptr));

    parameters.push_back(std::make_unique<Parameter>(String("sharp"), String("Sharp"), String(),
                                                     NormalisableRange<float>(0.f, 1.f, 0.001f, 0.5f),
                                                     0.5f, sharpSliderValueToText, sharpSliderTextToValue));
    
    parameters.push_back(std::make_unique<Parameter>(String("mod"), String("Mod"), String(),
                                                     NormalisableRange<float>(0.f, 1.f, 0.001f, 0.5f),
                                                     0.5f, sharpSliderValueToText, sharpSliderTextToValue));

    parameters.push_back(std::make_unique<Parameter>(String("gain"), String("Gain"), String(),
                                                     NormalisableRange<float>(0.001f, 7.94f, 0.001f, 0.3f),
                                                     1.0f, gainSliderValueToText, gainSliderTextToValue));

    return { parameters.begin(), parameters.end() };
}

//==============================================================================
waylosynth2::waylosynth2()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    parameters (*this, nullptr, Identifier(JucePlugin_Name), createParameterLayout())
{

    for (auto i = 0; i < numberOfVoices; ++i)
        synthesiser.addVoice(new MySynthesiserVoice());

    synthesiser.addSound(new MySynthesiserSound());

    attackParameter = parameters.getRawParameterValue("attack");
    decayParameter = parameters.getRawParameterValue("decay");
    sustainParameter = parameters.getRawParameterValue("sustain");
    releaseParameter = parameters.getRawParameterValue("release");
    typeParameter = parameters.getRawParameterValue("type");
    sharpParameter = parameters.getRawParameterValue("sharp");
    modParameter = parameters.getRawParameterValue("mod");
    gainParameter = parameters.getRawParameterValue("gain");
}

waylosynth2::~waylosynth2()
{
}

//==============================================================================
const String waylosynth2::getName() const
{
    return JucePlugin_Name;
}

bool waylosynth2::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool waylosynth2::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool waylosynth2::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double waylosynth2::getTailLengthSeconds() const
{
    return 0.0;
}

int waylosynth2::getNumPrograms()
{
return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int waylosynth2::getCurrentProgram()
{
    return 0;
}

void waylosynth2::setCurrentProgram (int index)
{
}

const String waylosynth2::getProgramName (int index)
{
    return {};
}

void waylosynth2::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void waylosynth2::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    keyboardState.reset();
    synthesiser.setCurrentPlaybackSampleRate(sampleRate);

}

void waylosynth2::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    keyboardState.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool waylosynth2::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void waylosynth2::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);
    

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    synthesiser.setEnvelopeParameters(ADSR::Parameters {*attackParameter, *decayParameter, *sustainParameter, *releaseParameter});
    synthesiser.setWavetypeParameter((int)*typeParameter);
    synthesiser.setSharpParameter(*sharpParameter);
    synthesiser.setModParameter(*modParameter);
    synthesiser.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    buffer.applyGainRamp(0, buffer.getNumSamples(), lastGain, *gainParameter);
    

    lastGain = *gainParameter;
}

//==============================================================================
bool waylosynth2::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* waylosynth2::createEditor()
{
    return new waylosynth2AudioProcessorEditor (*this, parameters);
}

//==============================================================================
void waylosynth2::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void waylosynth2::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new waylosynth2();
}
