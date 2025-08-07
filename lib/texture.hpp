//
// Created by James Pickering on 9/1/24.
//

#pragma once

#include <string>

struct TextureLoadOptions {
	bool hasAlpha = true;
	bool flip = true;
	bool repeat = false;
};

struct Texture {
	static auto fromPng(const std::string& path, const TextureLoadOptions& req) -> unsigned int;
	static auto fromArray(const float* array, int size) -> unsigned int;
};

auto textureFromResource(const std::string& resource, const TextureLoadOptions& opt) -> unsigned int;
auto textureFromBuffer(const void* data, int size, const TextureLoadOptions& opt) -> unsigned int;

auto setTextureData(unsigned int id, const float* data, int size) -> void;
