//
// Created by James Pickering on 7/24/25.
//

#include "engine.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <complex>
#include <iostream>
#include <valarray>
#include <vector>

using c_val_array = std::valarray<std::complex<float>>;

auto fft(c_val_array& x) {
	const size_t N = x.size();
	if (N <= 1)
		return;

	auto even = c_val_array{x[std::slice(0, N / 2, 2)]};
	auto odd = c_val_array{x[std::slice(1, N / 2, 2)]};

	fft(even);
	fft(odd);

	for (auto k = 0; k < N / 2; ++k) {
		auto t = std::polar<float>(1.0, -2.0 * M_PI * k / N) * odd[k];
		x[k] = even[k] + t;
		x[k + N / 2] = even[k] - t;
	}
}

auto ifft(c_val_array& x) {
	x = x.apply(std::conj);	 // conjugate input
	fft(x);					 // forward FFT
	x = x.apply(std::conj);	 // conjugate again
	x /= x.size();			 // normalize
}

auto stdArr(const c_val_array& data,
			const float mean,
			const bool sample = false) {
	const auto n = data.size();
	if (n <= (sample ? 1 : 0))
		return 0.0;

	auto variance = 0.0;
	for (const auto& x : data)
		variance += (x.real() - mean) * (x.real() - mean);

	variance /= (sample ? (n - 1) : n);

	return std::sqrt(variance);
}

auto genFreqBins(int n) -> std::vector<float> {
	auto vec = std::vector<float>{};
	if (n % 2 != 0) {
		--n;
	}
	const auto vecSize = n / 2 + 1;
	vec.reserve(vecSize);

	for (auto i = 0; i < vecSize; ++i) {
		vec.emplace_back(i);
	}

	return vec;
}

auto randVals = std::vector<float>{};

auto recalcRandVals() -> void {
	const auto seed = time(nullptr);
	srand(seed);

	randVals.clear();

	// TODO: 100 is way higher than the max but lets go no magic number
	for (auto i = 0; i < 100; ++i) {
		randVals.emplace_back((float)rand() / RAND_MAX * M_PI * 2.);
	}
}

auto genFractalOffsets(const int n, const float alpha, const float std)
	-> FractalNoiseResult {
	auto res = FractalNoiseResult{};

	if (randVals.empty()) {
		recalcRandVals();
	}

	res.frequencies = genFreqBins(n);
	res.frequencies[0] = config::Epsilon;  // The first bin will be 0 so we want
										   // to avoid a div by 0 error

	const auto numFreqs = res.frequencies.size();

	res.spectrum = std::vector<float>{};
	res.spectrum.reserve(numFreqs);

	for (auto i = 0; i < numFreqs; ++i) {
		res.spectrum.emplace_back(1. / std::pow(res.frequencies[i], alpha));
	}

	auto phases = std::vector<float>{};
	phases.reserve(numFreqs);

	for (auto i = 0; i < numFreqs; ++i) {
		phases.emplace_back(randVals[i]);
	}

	auto spectrum = std::vector<std::complex<float>>{};
	spectrum.reserve(numFreqs);

	for (auto i = 0; i < numFreqs; ++i) {
		spectrum.emplace_back(
			std::polar(std::sqrt(res.spectrum[i]), phases[i]));
	}

	auto fullSpectrum = std::vector<std::complex<float>>{};
	fullSpectrum.reserve(numFreqs * 2 - 1);

	for (auto i = 0; i < numFreqs; ++i) {
		fullSpectrum.emplace_back(spectrum[i]);
	}

	for (auto i = 1; i < numFreqs - 1; ++i) {
		fullSpectrum.emplace_back(std::conj(spectrum[numFreqs - i]));
	}

	auto valArr = c_val_array{fullSpectrum.data(), fullSpectrum.size()};

	ifft(valArr);

	auto sum = 0.;
	for (auto i = 0; i < valArr.size(); ++i) {
		sum += valArr[i].real();
	}
	const auto mean = sum / (float)valArr.size();

	valArr -= mean;
	valArr *= std / stdArr(valArr, mean);

	res.offsets = std::vector<float>{};
	for (auto i = 0; i < valArr.size(); ++i) {
		res.offsets.emplace_back(valArr[i].real());
	}

	const auto [minIt, maxIt] =
		std::minmax_element(res.offsets.begin(), res.offsets.end());
	const auto range = *maxIt - *minIt;
	res.minOffset = *minIt;

	for (const auto val : res.offsets) {
		res.normOffsets.push_back((val - *minIt) / range);
	}

	return res;
}

auto getOffsetAt(const State& ctx, const int idx) -> float {
	return (ctx.offsets.offsets.empty() || idx < 0 ||
			idx >= ctx.offsets.offsets.size())
			   ? 0
			   : (ctx.offsets.offsets.at(idx) -
				  ctx.offsets.minOffset * (1.f - ctx.lookahead)) *
					 (ctx.variance * 100.f + 1.f);
}

auto getOffsetAtI(const State& ctx, const int idx) -> int {
	return (int)std::round(getOffsetAt(ctx, idx));
}

auto applyState(State& state) -> void {
	state.stepsI = stepsFromKnobValue(state.steps);

	state.offsets =
		genFractalOffsets(state.stepsI, state.alpha.load() * 0.3f + 0.5f, 10);
	// state.minOffset = state.noise.minOffset;
	// state.vals = state.noise.offsets;
	state.alphaChanged = false;
	state.stepsChanged = false;
}

auto stepsFromKnobValue(const float value) -> int {
	return (int)(value * 20.f + 10.f);
}
