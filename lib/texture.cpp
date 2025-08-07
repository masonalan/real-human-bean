//
// Created by James Pickering on 9/1/24.
//

#include "texture.hpp"
#include "graphics.hpp"

#include <juce_opengl/juce_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <BinaryData.h>
#include <stb_image.h>

#include <iostream>
#include <ostream>

using namespace juce::gl;

auto Texture::fromPng(const std::string& path, const TextureLoadOptions& req)
	-> unsigned int {
	auto width = int{};
	auto height = int{};
	auto numChannels = int{};

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	stbi_set_flip_vertically_on_load(req.flip);

	const auto data = stbi_load(path.c_str(), &width, &height, &numChannels, 0);
	if (!data) {
		std::cerr << "Failed to load texture: \"" << path << "\"" << std::endl;
		stbi_image_free(data);
		return 0;
	}

	auto id = (unsigned int){};

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, req.hasAlpha ? GL_RGBA : GL_RGB, width,
				 height, 0, req.hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE,
				 data);
	if (req.repeat) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);

	std::cout << "Loaded texture: " << path << std::endl;

	return id;
}

auto Texture::fromArray(const float* array, const int size) -> unsigned int {
	auto id = 0u;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &id);
	checkError();
	glBindTexture(GL_TEXTURE_1D, id);
	checkError();
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, size, 0, GL_RED, GL_FLOAT, array);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	checkError();
	glBindTexture(GL_TEXTURE_1D, 0);
	checkError();

	return id;
}

auto textureFromResource(const std::string& resource,
						 const TextureLoadOptions& opt) -> unsigned int {
	auto size = 0;
	const auto data = BinaryData::getNamedResource(resource.c_str(), size);
	if (!data) {
		std::cerr << "Failed to load texture: \"" << resource << "\""
				  << std::endl;
		return 0;
	}
	return textureFromBuffer(data, size, opt);
}

auto textureFromBuffer(const void* buffer,
					   const int size,
					   const TextureLoadOptions& req) -> unsigned int {
	auto width = int{};
	auto height = int{};
	auto numChannels = int{};

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	stbi_set_flip_vertically_on_load(req.flip);

	const auto data = stbi_load_from_memory((stbi_uc const*)(buffer), size,
											&width, &height, &numChannels, 0);

	if (!data) {
		stbi_image_free(data);
		return 0;
	}

	auto id = (unsigned int){};

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, req.hasAlpha ? GL_RGBA : GL_RGB, width,
				 height, 0, req.hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE,
				 data);
	if (req.repeat) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);

	std::cout << "Loaded texture: " << "yayy" << std::endl;

	return id;
}

auto setTextureData(unsigned int id, const float* data, int size) -> void {
	glBindTexture(GL_TEXTURE_1D, id);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, size, 0, GL_RED, GL_FLOAT, data);
	glBindTexture(GL_TEXTURE_1D, 0);
}
