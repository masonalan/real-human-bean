//
// Created by James Pickering on 3/26/25.
//

#pragma once

#include "lib/engine.hpp"

#include <juce_audio_processors/juce_audio_processors.h>

class Processor : public juce::AudioProcessor {
   public:
	Processor();
	~Processor() override;

	auto prepareToPlay(double sampleRate, int samplesPerBlock) -> void override;
	auto releaseResources() -> void override;
	auto isBusesLayoutSupported(const BusesLayout& layouts) const
		-> bool override;

	auto processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&)
		-> void override;
	auto hasEditor() const -> bool override;

	auto createEditor() -> juce::AudioProcessorEditor* override;
	auto getName() const -> const juce::String override;

	auto acceptsMidi() const -> bool override;
	auto producesMidi() const -> bool override;
	auto isMidiEffect() const -> bool override;
	auto getTailLengthSeconds() const -> double override;

	auto getNumPrograms() -> int override;
	auto getCurrentProgram() -> int override;
	auto setCurrentProgram(int index) -> void override;
	auto getProgramName(int index) -> const juce::String override;
	auto changeProgramName(int index, const juce::String& newName)
		-> void override;

	auto getStateInformation(juce::MemoryBlock& destData) -> void override;
	auto setStateInformation(const void* data, int sizeInBytes)
		-> void override;

	State ctx;

	bool hasSetStateInfo = false;

   private:
	juce::AudioProcessorValueTreeState params;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Processor)

	juce::AudioBuffer<float> _delayBuffer;
	int _offset = 0;
	int _difference = 1;
	// int _writeOffset = 0;
	// int _readOffset = 1;
	bool _firstProcess = true;
	int _feedback = 5;
	int _sampleRate = 0;

	int _idx = -1;
};