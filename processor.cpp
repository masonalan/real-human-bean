//
// Created by James Pickering on 3/26/25.
//

#include "processor.hpp"
#include "editor.hpp"

#include "lib/controls.hpp"
#include "lib/engine.hpp"
#include "lib/serialize.hpp"

#include <assert.h>

constexpr auto cutoff = 0.00001f;
constexpr auto attack = 50;

auto paramFloat(const std::string& name, float defaultValue)
	-> std::unique_ptr<juce::AudioParameterFloat> {
	return std::make_unique<juce::AudioParameterFloat>(
		juce::ParameterID{name, Version}, name, 0.f, 1.f, defaultValue);
}

Processor::Processor()
	: AudioProcessor{BusesProperties{}
						 .withInput("Input",
									juce::AudioChannelSet::stereo(),
									true)
						 .withOutput("Output",
									 juce::AudioChannelSet::stereo(),
									 true)},
	  params{*this,
			 nullptr,
			 juce::Identifier{"real-human-bean-vst"},
			 {paramFloat("alpha", 0.27f), paramFloat("steps", 0.4f),
			  paramFloat("variance", 0.78f), paramFloat("lookahead", 0.5f)}},
	  ctx{} {
	// ctx.alpha = params.getRawParameterValue("alpha");
}

Processor::~Processor() {
	std::cout << "yunkkkk" << std::endl;
};

auto Processor::getName() const -> const juce::String {
	return JucePlugin_Name;
}

auto Processor::acceptsMidi() const -> bool {
	return false;
}

auto Processor::producesMidi() const -> bool {
	return false;
}

auto Processor::isMidiEffect() const -> bool {
	return false;
}

auto Processor::getTailLengthSeconds() const -> double {
	return 0.0;
}

auto Processor::getNumPrograms() -> int {
	return 1;
}

auto Processor::getCurrentProgram() -> int {
	return 0;
}

auto Processor::setCurrentProgram(int index) -> void {
	juce::ignoreUnused(index);
}

auto Processor::getProgramName(int index) -> const juce::String {
	juce::ignoreUnused(index);
	return {};
}

auto Processor::changeProgramName(int index, const juce::String& newName)
	-> void {
	juce::ignoreUnused(index);
}

auto Processor::prepareToPlay(const double sampleRate,
							  const int samplesPerBlock) -> void {
	_sampleRate = static_cast<int>(sampleRate);
	_delayBuffer.setSize(2, _sampleRate);
	_delayBuffer.clear();
}

auto Processor::releaseResources() -> void {
	std::cout << "releasing resources" << std::endl;
}

auto Processor::isBusesLayoutSupported(const BusesLayout& layouts) const
	-> bool {
	return true;
}

auto Processor::processBlock(juce::AudioBuffer<float>& buffer,
							 juce::MidiBuffer& midiMessages) -> void {
	juce::ignoreUnused(midiMessages);

	const auto totalNumInputChannels = getTotalNumInputChannels();
	const auto totalNumOutputChannels = getTotalNumOutputChannels();

	// Clear any channels we are not using/updating
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
		buffer.clear(i, 0, buffer.getNumSamples());
	}

	// For each channel, get the value in the buffer at i.
	// We have a delay buffer that we start writing at
	for (auto channel = 0; channel < totalNumInputChannels; ++channel) {
		const auto buffPtr = buffer.getWritePointer(channel);

		for (auto i = 0; i < buffer.getNumSamples(); ++i) {
			const auto sample = buffPtr[i];

			// Check to see if we have begun playing a sound, and if so, set a
			// flag saying so and reset our counter for the gate
			const auto lastSampleAbs = std::fabs(ctx._lastSample.at(channel));
			const auto sampleAbs = std::fabs(sample);
			if (!ctx.isSoundOccurring.at(channel) && lastSampleAbs < cutoff &&
				sampleAbs > cutoff) {
				ctx.isSoundOccurring.at(channel) = true;
				ctx.gateIdx.at(channel) = 0;

				std::cout << "Start sound on channel " << channel << std::endl;
			}

			// If the sound is occurring, check to see if the sound has ended
			// (which would be some amount of consecutive samples below a
			// threshold)
			if (ctx.isSoundOccurring.at(channel)) {
				if (sampleAbs < cutoff) {
					++ctx.gateIdx.at(channel);
					if (++ctx.gateIdx.at(channel) >= attack) {
						ctx.isSoundOccurring.at(channel) = false;
						ctx.currDelay = (ctx.currDelay + 1) % ctx.stepsI;

						std::cout << "End sound on channel " << channel
								  << std::endl;
					}
				} else {
					ctx.gateIdx.at(channel) = 0;
				}
			}

			const auto offset = getOffsetAtI(ctx, ctx.currDelay);
			ctx.delayBuffer.at(channel).at((ctx.delayIdx.at(channel) + offset) %
										   delayBufferSize) += sample;

			buffPtr[i] =
				ctx.delayBuffer.at(channel).at(ctx.delayIdx.at(channel));
			ctx.delayBuffer.at(channel).at(ctx.delayIdx.at(channel)) = 0;

			ctx.delayIdx.at(channel) =
				(ctx.delayIdx.at(channel) + 1) % delayBufferSize;
		}
	}
}

auto Processor::hasEditor() const -> bool {
	return true;
}

auto Processor::createEditor() -> juce::AudioProcessorEditor* {
	return new Editor(*this);
}

auto Processor::getStateInformation(juce::MemoryBlock& destData) -> void {
	serialize(destData, ctx);
}

auto Processor::setStateInformation(const void* data, const int sizeInBytes)
	-> void {
	deserialize(data, sizeInBytes, ctx);
}

auto JUCE_CALLTYPE createPluginFilter() -> juce::AudioProcessor* {
	return new Processor();
}