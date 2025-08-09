//
// Created by James Pickering on 8/5/25.
//

#pragma once

#include <juce_core/juce_core.h>

struct State;

auto serialize(juce::MemoryBlock& block, State& context) -> void;
auto deserialize(const void* data, int sizeInBytes, State& context) -> void;
