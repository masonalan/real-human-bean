//
// Created by James Pickering on 7/24/25.
//

#pragma once

#include <glm/glm.hpp>

#include <atomic>
#include <vector>

constexpr auto delayBufferSize = 10000;
constexpr auto Epsilon = 0.001f;
constexpr auto Version = 2;
constexpr auto Version_1 = 1;
constexpr auto Version_2 = 2;

struct FractalNoiseResult {
	std::vector<float> frequencies;
	std::vector<float> spectrum;
	std::vector<float> offsets;
	std::vector<float> normOffsets;
	float minOffset;
};

struct State {
	std::atomic<float> alpha = 0.5f;
	std::atomic<float> steps = 0.5f;
	std::atomic<float> variance = 0.5f;
	std::atomic<float> lookahead = 0.0f;

	std::array<int, 2> delayIdx = {0, 0};
	std::array<int, 2> gateIdx{};
	std::array<bool, 2> isSoundOccurring{false, false};
	std::array<bool, 2> isPlayingSample{false, false};
	std::array<float, 2> _lastSample{};

	std::array<std::array<float, delayBufferSize>, 2> delayBuffer{};

	std::atomic<int> stepsI;
	std::atomic<bool> queuedSeedRecalc = false;
	std::atomic<bool> queuedOffsetRecalc = false;
	std::atomic<bool> eventOffsetsUpdated = false;

	FractalNoiseResult offsets;

	int currDelay = -1;
};

auto applyState(State& state) -> void;

auto stepsFromKnobValue(float value) -> int;

namespace config {
constexpr auto NumNoiseSamples = 100;
constexpr auto NoiseSampleRange = 10;
constexpr auto NoiseSampleStep =
	(float)NoiseSampleRange / (float)NumNoiseSamples;
constexpr auto Epsilon = 0.0001f;
constexpr auto WindowSize = glm::vec2{1000, 500};
constexpr auto HalfWindowSize = WindowSize / 2.f;
}  // namespace config

auto genFractalOffsets(int n, float alpha, float std) -> FractalNoiseResult;
auto getOffsetAt(const State& ctx, int idx) -> float;
auto getOffsetAtI(const State& ctx, int idx) -> int;
auto recalcRandVals() -> void;