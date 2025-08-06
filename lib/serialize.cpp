//
// Created by James Pickering on 8/5/25.
//

#include "serialize.hpp"
#include "engine.hpp"

#include <juce_core/juce_core.h>

auto serialize(juce::MemoryBlock& block, const State& context) -> void {
	auto stream = juce::MemoryOutputStream{block, false};
	stream.writeInt(Version);

	if constexpr (Version == Version_1) {
		stream.writeFloat(context.alpha.load());
	} else if constexpr (Version == Version_2) {
		stream.writeFloat(context.alpha.load());
		stream.writeFloat(context.steps.load());
		stream.writeFloat(context.variance.load());
		stream.writeFloat(context.lookahead.load());
	} else {
		throw std::runtime_error{"serialize(): Unsupported version " + Version};
	}
}

auto deserialize(const void* data, const int sizeInBytes, State& context)
	-> void {
	auto stream = juce::MemoryInputStream{data, (size_t)sizeInBytes, false};
	const auto version = stream.readInt();

	if (version == Version_1) {
		context.alpha.store(stream.readFloat());
	} else if (version == Version_2) {
		context.alpha.store(stream.readFloat());
		context.steps.store(stream.readFloat());
		context.variance.store(stream.readFloat());
		context.lookahead.store(stream.readFloat());

		applyState(context);
	} else {
		std::cerr << "deserialize(): Unsupported version " + version
				  << std::endl;
	}
}