/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AutocatAudioProcessor::AutocatAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
	parameters(*this, nullptr, juce::Identifier("AutoCAT"),
	{
		std::make_unique<juce::AudioParameterInt>("n_notes", "Density", 1, 10, 4),
		std::make_unique<juce::AudioParameterBool>("clear_midi", "Discard Notes", false),
		std::make_unique<juce::AudioParameterBool>("sync_time", "Synch Time", true),
		std::make_unique<juce::AudioParameterFloat>("staccato", "Staccato", 0.0, 1.0, 0.2),
		std::make_unique<juce::AudioParameterInt>("velocity", "Velocity", 0, 127, 100),
		std::make_unique<juce::AudioParameterInt>("octave", "Octave", -1, 6, 3)
	})
{
	nNotes = (juce::AudioParameterInt*) parameters.getParameter("n_notes");
	clearMidiBuffer = (juce::AudioParameterBool*) parameters.getParameter("clear_midi");
	syncTime = (juce::AudioParameterBool*) parameters.getParameter("sync_time");
	staccato = (juce::AudioParameterFloat*) parameters.getParameter("staccato");
	velocity = (juce::AudioParameterInt*) parameters.getParameter("velocity");
	octave = (juce::AudioParameterInt*) parameters.getParameter("octave");
}

AutocatAudioProcessor::~AutocatAudioProcessor()
{
}

//==============================================================================
const juce::String AutocatAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AutocatAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AutocatAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AutocatAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AutocatAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AutocatAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AutocatAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AutocatAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AutocatAudioProcessor::getProgramName (int index)
{
    return {};
}

void AutocatAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AutocatAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
	rate = sampleRate;
	time = 0;
	notes.clear();
	chordNotes.clear();
}

void AutocatAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AutocatAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void AutocatAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

	// Processing the MIDI information:
	for (const auto metadata : midiMessages) {
		const auto msg = metadata.getMessage();
		if (msg.isNoteOn()) notes.add(msg.getNoteNumber() % 12);
	}

	if (*clearMidiBuffer) midiMessages.clear();

	int nSamps = buffer.getNumSamples();
	juce::AudioPlayHead::CurrentPositionInfo positionInfo;
	getPlayHead()->getCurrentPosition(positionInfo);

	float noteDurQ = ((float)(positionInfo.timeSigNumerator) / (float)(positionInfo.timeSigDenominator)) * 4.0;
	float noteDurS = (noteDurQ * 60) / positionInfo.bpm;
	int noteDur = std::ceil(noteDurS * rate);

	if (*syncTime) time = (positionInfo.timeInSamples - nSamps) % noteDur;

	auto offset = juce::jlimit(0, nSamps - 1, noteDur - time);

	if ((time + nSamps) >= noteDur) {
		for (auto note : chordNotes) {
			midiMessages.addEvent(juce::MidiMessage::noteOff(1, note), offset);
		}
		chordNotes.clear();

		for (int i = 0; i < *nNotes && notes.size() > 0; i++) {
			int noteIdx = std::rand() % notes.size();
			int note = notes[noteIdx];
			notes.remove(note);
			int realNote = note + (*octave + 1) * 12 + (std::rand() % 2) * 12;
			midiMessages.addEvent(juce::MidiMessage::noteOn(1, realNote, (juce::uint8) *velocity), offset);
			chordNotes.add(realNote);
		}

		notes.clear();
		for (auto note : chordNotes) notes.add(note % 12);
	}
	else if ((time + nSamps) >= (noteDur * (1.0 - *staccato))) {
		for (auto note : chordNotes) {
			midiMessages.addEvent(juce::MidiMessage::noteOff(1, note), offset);
		}
	}

	time = (time + nSamps) % noteDur;
}

//==============================================================================
bool AutocatAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AutocatAudioProcessor::createEditor()
{
    return new AutocatAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void AutocatAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
	auto state = parameters.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void AutocatAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(parameters.state.getType()))
			parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AutocatAudioProcessor();
}
