//
// Created by James Pickering on 7/21/25.
//

#pragma once

#include <glm/glm.hpp>

#include <string>

struct Shader {
	unsigned int id = 0;
	int flags = 0;
	bool loaded = false;
	std::string vertexPath;
	std::string fragmentPath;
};

struct GraphicsContext {
	unsigned int quadVertexBuffer = 0;
	unsigned int quadVertexArray = 0;
	unsigned int notesTex = 0;
	unsigned int notesFbo = 0;

	Shader shader;
	Shader noiseShader;
	Shader graphShader;
	Shader circleShader;

	unsigned int bgTextureId = 0;
	unsigned int powerTexId = 0;
	unsigned int offsetGraphTexId = 0;
	unsigned int knobTexId = 0;
	unsigned int normalTexId = 0;
	unsigned int shiftTexId = 0;
	unsigned int labelFig3Tex = 0;

	unsigned int reseedButtonTex;
	unsigned int reseedButtonHoveredTex;
	unsigned int reseedButtonPressedTex;
};

auto checkError() -> void;
auto setupGraphics(GraphicsContext& context) -> void;
auto setUniform(unsigned int shaderId,
				const std::string& uniformName,
				glm::vec2 value) -> void;
auto setUniform(unsigned int shaderId,
				const std::string& uniformName,
				bool value) -> void;
auto setUniform(unsigned int shaderId,
				const std::string& uniformName,
				int value) -> void;
auto setUniform(unsigned int shaderId,
				const std::string& uniformName,
				float value) -> void;
auto setUniform(unsigned int shaderId,
				const std::string& uniformName,
				const glm::mat4& value) -> void;
auto setUniform(unsigned int shaderId,
				const std::string& uniformName,
				const float* value,
				int size) -> void;
