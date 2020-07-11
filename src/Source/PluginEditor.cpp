/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AutocatAudioProcessorEditor::AutocatAudioProcessorEditor (AutocatAudioProcessor& p, juce::AudioProcessorValueTreeState& s)
    : AudioProcessorEditor (&p), audioProcessor (p), state(s)
{
	getLookAndFeel().setColour(juce::ResizableWindow::backgroundColourId, juce::Colour::fromRGB(220, 228, 230));
	getLookAndFeel().setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
	getLookAndFeel().setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::white);
	getLookAndFeel().setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(227, 186, 205));
	getLookAndFeel().setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(124, 194, 141));
	getLookAndFeel().setColour(juce::TextButton::textColourOffId, juce::Colour::fromRGB(46, 37, 89));
	getLookAndFeel().setColour(juce::TextButton::textColourOnId, juce::Colour::fromRGB(46, 37, 89));
	getLookAndFeel().setColour(juce::Label::textColourId, juce::Colour::fromRGB(46, 37, 89));

	setResizable(true, true);

	addAndMakeVisible(velocitySlider);
	velocitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
	velocitySlider.setRange(0, 127, 1);
	velocitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
	velocitySlider.setValue(100);
	addAndMakeVisible(velocityLabel);
	velocityLabel.setText("Velocity", juce::dontSendNotification);
	velocityLabel.attachToComponent(&velocitySlider, false);
	velocityLabel.setJustificationType(juce::Justification(juce::Justification::centredBottom));
	velocityAtch.reset(new SliderAttachment(state, "velocity", velocitySlider));

	addAndMakeVisible(densitySlider);
	densitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
	densitySlider.setRange(1, 10, 1);
	densitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
	densitySlider.setValue(4);
	addAndMakeVisible(densityLabel);
	densityLabel.setText("Density", juce::dontSendNotification);
	densityLabel.attachToComponent(&densitySlider, false);
	densityLabel.setJustificationType(juce::Justification(juce::Justification::centredBottom));
	densityAtch.reset(new SliderAttachment(state, "n_notes", densitySlider));

	addAndMakeVisible(staccatoSlider);
	staccatoSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
	staccatoSlider.setRange(0.0, 1.0, 0.01);
	staccatoSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
	staccatoSlider.setValue(0.2);
	addAndMakeVisible(staccatoLabel);
	staccatoLabel.setText("Staccato", juce::dontSendNotification);
	staccatoLabel.attachToComponent(&staccatoSlider, false);
	staccatoLabel.setJustificationType(juce::Justification(juce::Justification::centredBottom));
	staccatoAtch.reset(new SliderAttachment(state, "staccato", staccatoSlider));

	addAndMakeVisible(octaveSlider);
	octaveSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
	octaveSlider.setRange(-1, 6, 1);
	octaveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
	octaveSlider.setValue(3);
	addAndMakeVisible(octaveLabel);
	octaveLabel.setText("Octave", juce::dontSendNotification);
	octaveLabel.attachToComponent(&octaveSlider, false);
	octaveLabel.setJustificationType(juce::Justification(juce::Justification::centredBottom));
	octaveAtch.reset(new SliderAttachment(state, "octave", octaveSlider));

	addAndMakeVisible(discardToggle);
	discardToggle.setClickingTogglesState(true);
	discardToggle.setButtonText("Discard\nNotes");
	discardAtch.reset(new ButtonAttachment(state, "clear_midi", discardToggle));

	addAndMakeVisible(synchToggle);
	synchToggle.setClickingTogglesState(true);
	synchToggle.setToggleState(true, juce::dontSendNotification);
	synchToggle.setButtonText("Synch\nPlay");
	synchAtch.reset(new ButtonAttachment(state, "sync_time", synchToggle));

	logoImg = juce::Drawable::createFromSVG(*juce::parseXML(BinaryData::AutoCAT_svg));
	addAndMakeVisible(*logoImg);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 500);
}

AutocatAudioProcessorEditor::~AutocatAudioProcessorEditor()
{
}

//==============================================================================
void AutocatAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AutocatAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	auto logoY = 130;

	auto sliderPad = 20;
	auto sliderY = 250;
	auto sliderHeight = 120;
	auto sliderWidth = (getWidth() - 5 * sliderPad) / 4;
	
	auto toggleY = 400;
	auto togglePad = 80;
	auto toggleHeight = 60;
	auto toggleWidth = (getWidth() - 3 * togglePad) / 2;

	logoImg->setCentrePosition(getWidth() / 2, logoY);

	velocitySlider.setBounds(sliderPad, sliderY - 10, sliderWidth, sliderHeight);
	densitySlider.setBounds(sliderPad * 2 + sliderWidth, sliderY + 10, sliderWidth, sliderHeight);
	staccatoSlider.setBounds(sliderPad * 3 + sliderWidth * 2, sliderY + 10, sliderWidth, sliderHeight);
	octaveSlider.setBounds(sliderPad * 4 + sliderWidth * 3, sliderY - 10, sliderWidth, sliderHeight);

	discardToggle.setBounds(togglePad, toggleY, toggleWidth, toggleHeight);
	synchToggle.setBounds(togglePad * 2 + toggleWidth, toggleY, toggleWidth, toggleHeight);
}
