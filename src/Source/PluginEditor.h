/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class AutocatAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AutocatAudioProcessorEditor (AutocatAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~AutocatAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AutocatAudioProcessor& audioProcessor;
	juce::AudioProcessorValueTreeState& state;

	juce::Slider velocitySlider;
	juce::Slider densitySlider;
	juce::Slider staccatoSlider;
	juce::Slider octaveSlider;
	juce::Label velocityLabel;
	juce::Label densityLabel;
	juce::Label staccatoLabel;
	juce::Label octaveLabel;
	juce::TextButton discardToggle;
	juce::TextButton synchToggle;
	std::unique_ptr<juce::Drawable> logoImg;

	std::unique_ptr<SliderAttachment> velocityAtch;
	std::unique_ptr<SliderAttachment> densityAtch;
	std::unique_ptr<SliderAttachment> staccatoAtch;
	std::unique_ptr<SliderAttachment> octaveAtch;
	std::unique_ptr<ButtonAttachment> discardAtch;
	std::unique_ptr<ButtonAttachment> synchAtch;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutocatAudioProcessorEditor)
};
