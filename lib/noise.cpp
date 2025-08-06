//
// Created by James Pickering on 6/26/25.
//

#include "noise.hpp"

#include <array>
#include <cmath>

constexpr auto maxIndex = 255;
constexpr auto permutation = std::array<int, 256>{
	151, 160, 137, 91,	90,	 15,  131, 13,	201, 95,  96,  53,	194, 233, 7,
	225, 140, 36,  103, 30,	 69,  142, 8,	99,	 37,  240, 21,	10,	 23,  190,
	6,	 148, 247, 120, 234, 75,  0,   26,	197, 62,  94,  252, 219, 203, 117,
	35,	 11,  32,  57,	177, 33,  88,  237, 149, 56,  87,  174, 20,	 125, 136,
	171, 168, 68,  175, 74,	 165, 71,  134, 139, 48,  27,  166, 77,	 146, 158,
	231, 83,  111, 229, 122, 60,  211, 133, 230, 220, 105, 92,	41,	 55,  46,
	245, 40,  244, 102, 143, 54,  65,  25,	63,	 161, 1,   216, 80,	 73,  209,
	76,	 132, 187, 208, 89,	 18,  169, 200, 196, 135, 130, 116, 188, 159, 86,
	164, 100, 109, 198, 173, 186, 3,   64,	52,	 217, 226, 250, 124, 123, 5,
	202, 38,  147, 118, 126, 255, 82,  85,	212, 207, 206, 59,	227, 47,  16,
	58,	 17,  182, 189, 28,	 42,  223, 183, 170, 213, 119, 248, 152, 2,	  44,
	154, 163, 70,  221, 153, 101, 155, 167, 43,	 172, 9,   129, 22,	 39,  253,
	19,	 98,  108, 110, 79,	 113, 224, 232, 178, 185, 112, 104, 218, 246, 97,
	228, 251, 34,  242, 193, 238, 210, 144, 12,	 191, 179, 162, 241, 81,  51,
	145, 235, 249, 14,	239, 107, 49,  192, 214, 31,  181, 199, 106, 157, 184,
	84,	 204, 176, 115, 121, 50,  45,  127, 4,	 150, 254, 138, 236, 205, 93,
	222, 114, 67,  29,	24,	 72,  243, 141, 128, 195, 78,  66,	215, 61,  156,
	180};
constexpr auto p = [] {
	auto arr = std::array<int, 512>{};
	for (auto i = 0; i < 512; i++) {
		arr[i] = permutation[i % 256];
	}
	return arr;
}();

static auto repeat = 0;

/**
 * Increments an index and repeating if necessary
 */
inline auto inc(int num) {
	num++;
	if (repeat > 0) {
		num %= repeat;
	}
	return num;
}

/**
 * Eases coordinate values so that they fade toward integral values
 * 6t^5 - 15t^4 + 10t^3 but written faster and harder to read because
 * that's what algorithm people are: illiterate so they make normal
 * people who read words and math pay.
 */
inline auto fade(const double t) -> double {
	return t * t * t * (t * (t * 6. - 15.) + 10.);
}

/**
 * Calculates the dot product of a randomly selected gradient vector and the 8
 * location vectors. Uses "fancy bit-flipping" code to do it. The 8 location
 * vectors are specified in the article linked below and are hand-picked by
 * Kevin himself.
 */
inline auto grad(const int hash,
				 const double x,
				 const double y,
				 const double z) {
	int h = hash & 15;		   // CONVERT LO 4 BITS OF HASH CODE
	double u = h < 8 ? x : y,  // INTO 12 GRADIENT DIRECTIONS.
		v = h < 4				 ? y
			: h == 12 || h == 14 ? x
								 : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

/**
 * Linear interpolation
 */
inline auto lerp(const double a, const double b, const double x) {
	return a + x * (b - a);
}

/**
 * Improved Perlin Noise
 *
 * Used this article as a resource:
 * https://adrianb.io/2014/08/09/perlinnoise.html
 *
 * Overview:
 *  1. Divide x, y, and z coordinates into integral unit cubes to find the
 * coordinate's location within that cube.
 *  2. On each of the 8 unit coords (vertices of cube), pick vectors from the
 * point in the center of the cube to the edges of the cube (in non-improved
 * version these were pseudorandom vectors). Read about it here:
 * https://mrl.cs.nyu.edu/~perlin/paper445.pdf
 *  3. Next, calculate the vectors from the vertices to the point x, y, z.
 *  4. Take the dot product between the two vectors from each vertex. This gives
 * us our final influence values. If the result of the dot product is positive
 * then it is in the direction of the gradient (what is it???) otherwise the
 * direction is against the gradient. (James here, and honestly at this point
 * I've become quite lost...)
 *  5. Interpolate between these 8 influence values. Average the averages! Easy!
 *  6. We are pretty much done, but linear interpolation looks unnatural (while
 * being computationally cheap). So, we use an ease curve that has been
 * specified by Kevin Perlin himself, the Truth, and the Light of all mankind.
 * It appears to be the sort of polynomial a 2-CB user might come up with.
 *  7. And that is it...
 */
auto perlin(double x, double y, double z) -> double {
	if (repeat > 0) {
		// Change coordinates to their "local" repetitions
		x = fmod(x, repeat);
		y = fmod(y, repeat);
		z = fmod(z, repeat);
	}

	// Calculate the unit cube (left bounds). We bind our coords to the range
	// [0, 255]. Perlin noise always repeats every 256 coords.
	const auto xi = (int)std::floor(x) & maxIndex;
	const auto yi = (int)std::floor(y) & maxIndex;
	const auto zi = (int)std::floor(z) & maxIndex;

	const auto xf = x - std::floor(x);
	const auto yf = y - std::floor(y);
	const auto zf = z - std::floor(z);

	const auto u = fade(xf);
	const auto v = fade(yf);
	const auto w = fade(zf);

	// Hashes all 8 unit cube coordinates surrounding the input coordinate.
	// "inc" is only used to make sure the hash value repeats. The value
	// of this hash function is [0, 255].
	const auto aaa = p[p[p[xi] + yi] + zi];
	const auto aba = p[p[p[xi] + inc(yi)] + zi];
	const auto aab = p[p[p[xi] + yi] + inc(zi)];
	const auto abb = p[p[p[xi] + inc(yi)] + inc(zi)];
	const auto baa = p[p[p[inc(xi)] + yi] + zi];
	const auto bba = p[p[p[inc(xi)] + inc(yi)] + zi];
	const auto bab = p[p[p[inc(xi)] + yi] + inc(zi)];
	const auto bbb = p[p[p[inc(xi)] + inc(yi)] + inc(zi)];

	int a = p[xi] + yi, aa = p[a] + zi,
		ab = p[a + 1] + zi,	 // HASH COORDINATES OF
		b = p[xi + 1] + yi, ba = p[b] + zi,
		bb = p[b + 1] + zi;	 // THE 8 CUBE CORNERS,

	auto x1 = lerp(grad(aaa, xf, yf, zf), grad(baa, xf - 1.f, yf, zf), u);
	auto x2 =
		lerp(grad(aba, xf, yf - 1.f, zf), grad(bba, xf - 1, yf - 1.f, zf), u);
	const auto y1 = lerp(x1, x2, v);

	x1 =
		lerp(grad(aab, xf, yf, zf - 1.f), grad(bab, xf - 1.f, yf, zf - 1.f), u);
	x2 = lerp(grad(abb, xf, yf - 1.f, zf - 1.f),
			  grad(bbb, xf - 1.f, yf - 1.f, zf - 1.f), u);
	const auto y2 = lerp(x1, x2, v);

	return (lerp(y1, y2, w) + 1.f) / 2.f;
}

auto octavePerlin(const glm::vec<3, double> point,
				  const int octaves,
				  const double persistence) -> double {
	auto total = 0.;
	auto freq = 1.;
	auto amp = 1.;
	auto maxVal = 0.;

	for (auto i = 0; i < octaves; i++) {
		total += perlin(point.x * freq, point.y * freq, point.z * freq) * amp;
		maxVal += amp;
		amp *= persistence;
		freq *= 2.;
	}

	return total / maxVal;
}

auto octavePerlinSigned(const glm::vec<3, double> point,
						const int octaves,
						const double persistence) -> double {
	return octavePerlin(point, octaves, persistence) * 2. - 1.;
}
