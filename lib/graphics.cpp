//
// Created by James Pickering on 7/21/25.
//

#include "graphics.hpp"

#include <juce_opengl/juce_opengl.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>

#include "texture.hpp"

// auto quadVertexBuffer = 0u;
// auto quadVertexArray = 0u;
// auto shader = Shader{};
// auto noiseShader = Shader{};
// auto graphShader = Shader{};
// auto circleShader = Shader{};
// auto notesFbo = 0u;
// auto notesTex = 0u;

using namespace juce::gl;

auto loadShader(Shader& shader) -> void {
	if (shader.loaded) {
		return;
	}

	const auto vertexPath = shader.vertexPath.c_str();
	const auto fragmentPath = shader.fragmentPath.c_str();

	auto vCode = std::string{};
	auto fCode = std::string{};
	auto vFile = std::ifstream{};
	auto fFile = std::ifstream{};

	try {
		vFile.open(vertexPath);
		fFile.open(fragmentPath);

		auto vStream = std::stringstream{};
		auto fStream = std::stringstream{};

		vStream << vFile.rdbuf();
		fStream << fFile.rdbuf();

		vCode = vStream.str();
		fCode = fStream.str();
	} catch (const std::ifstream::failure& e) {
		std::cout << "Shader(): " << e.what() << std::endl;
		throw e;
	}

	const auto vCodeCStr = vCode.c_str();
	const auto fCodeCStr = fCode.c_str();

	auto success = int{};
	auto&& log = (char[512]){};

	const auto vId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vId, 1, &vCodeCStr, nullptr);
	glCompileShader(vId);
	glGetShaderiv(vId, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(vId, 512, nullptr, log);
		std::cout << "Shader(): " << log << std::endl;
		throw std::runtime_error("Failed to compile vertex shader");
	}

	const auto fId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fId, 1, &fCodeCStr, nullptr);
	glCompileShader(fId);
	glGetShaderiv(fId, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(fId, 512, nullptr, log);
		std::cout << "Shader(): " << log << std::endl;
		throw std::runtime_error("Failed to compile fragment shader");
	}

	auto id = glCreateProgram();
	glAttachShader(id, vId);
	glAttachShader(id, fId);
	glLinkProgram(id);
	glGetProgramiv(id, GL_LINK_STATUS, &success);

	if (!success) {
		glGetProgramInfoLog(id, 512, nullptr, log);
		std::cout << "Shader(): " << log << std::endl;
		throw std::runtime_error("Failed to link shaders");
	}

	glDeleteShader(vId);
	glDeleteShader(fId);

	shader.id = id;
	shader.loaded = true;

	std::cout << "Loaded shader: " << shader.id << std::endl;
}

auto checkError() -> void {
	auto error = glGetError();
	switch (error) {
		case GL_INVALID_ENUM:
			std::cout << "GL_INVALID_ENUM" << std::endl;
			break;
		case GL_INVALID_OPERATION:
			std::cout << "GL_INVALID_OPERATION" << std::endl;
			break;
		case 0:
			return;
		default:
			std::cout << "Unknown error!" << std::endl;
			break;
	}
}

auto setupGraphics(GraphicsContext& context) -> void {
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	constexpr float vertices[] = {
		0.5f,  0.5f,  1, 1,	 // top right
		0.5f,  -0.5f, 1, 0,	 // bottom right
		-0.5f, 0.5f,  0, 1,	 // top left
		0.5f,  -0.5f, 1, 0,	 // bottom right
		-0.5f, -0.5,  0, 0,	 // bottom left
		-0.5f, 0.5f,  0, 1	 // top left
	};

	checkError();
	context.shader.vertexPath =
		"/Users/jamespickering/tyos/real human bean/shader.vert";
	context.shader.fragmentPath =
		"/Users/jamespickering/tyos/real human bean/shader.frag";
	loadShader(context.shader);
	checkError();

	context.noiseShader.vertexPath =
		"/Users/jamespickering/tyos/real human bean/shader.vert";
	context.noiseShader.fragmentPath =
		"/Users/jamespickering/tyos/real human bean/noise-shader.frag";
	loadShader(context.noiseShader);
	checkError();

	context.graphShader.vertexPath =
		"/Users/jamespickering/tyos/real human bean/shader.vert";
	context.graphShader.fragmentPath =
		"/Users/jamespickering/tyos/real human bean/graph-shader.frag";
	loadShader(context.graphShader);
	checkError();

	context.circleShader.vertexPath =
		"/Users/jamespickering/tyos/real human bean/shader.vert";
	context.circleShader.fragmentPath =
		"/Users/jamespickering/tyos/real human bean/circle-shader.frag";
	loadShader(context.circleShader);
	checkError();

	glUseProgram(context.shader.id);
	checkError();

	glGenBuffers(1, &context.quadVertexBuffer);
	checkError();
	glBindBuffer(GL_ARRAY_BUFFER, context.quadVertexBuffer);
	checkError();
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	checkError();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	checkError();

	glGenVertexArrays(1, &context.quadVertexArray);
	checkError();
	glBindVertexArray(context.quadVertexArray);
	checkError();
	glBindBuffer(GL_ARRAY_BUFFER, context.quadVertexBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, nullptr);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4,
						  (void*)(sizeof(float) * 2));
	glBindVertexArray(0);

	glGenTextures(1, &context.notesTex);
	glBindTexture(GL_TEXTURE_2D, context.notesTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1000 * 2, 500 * 2, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &context.notesFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, context.notesFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						   context.notesTex, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Framebuffer is not complete");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	context.powerTexId = Texture::fromArray(nullptr, 0);
	context.offsetGraphTexId = Texture::fromArray(nullptr, 0);

	auto opt = TextureLoadOptions{};
	opt.hasAlpha = true;
	opt.flip = true;

	context.bgTextureId = Texture::fromPng(
		"/Users/jamespickering/tyos/real human bean/bg3.png", opt);
	context.knobTexId = Texture::fromPng(
		"/Users/jamespickering/tyos/real human bean/knob2.png", opt);
	context.normalTexId = Texture::fromPng(
		"/Users/jamespickering/tyos/real human bean/normal.png", opt);
	context.shiftTexId = Texture::fromPng(
		"/Users/jamespickering/tyos/real human bean/shift.png", opt);
	context.fig3TexId = Texture::fromPng(
		"/Users/jamespickering/tyos/real human bean/fig3.png", opt);
	context.reseedButtonTex = Texture::fromPng(
		"/Users/jamespickering/tyos/real human bean/btn-play.png", opt);
	context.reseedButtonHoveredTex = Texture::fromPng(
		"/Users/jamespickering/tyos/real human bean/btn-play-hovered.png", opt);
	context.reseedButtonPressedTex = Texture::fromPng(
		"/Users/jamespickering/tyos/real human bean/btn-play-pressed.png", opt);
}

auto setUniform(const unsigned int shaderId,
				const std::string& uniformName,
				const glm::vec2 value) -> void {
	glUniform2f(glGetUniformLocation(shaderId, uniformName.c_str()), value.x,
				value.y);
}

auto setUniform(const unsigned int shaderId,
				const std::string& uniformName,
				const bool value) -> void {
	glUniform1i(glGetUniformLocation(shaderId, uniformName.c_str()), value);
}

auto setUniform(const unsigned int shaderId,
				const std::string& uniformName,
				const int value) -> void {
	glUniform1i(glGetUniformLocation(shaderId, uniformName.c_str()), value);
}

auto setUniform(const unsigned int shaderId,
				const std::string& uniformName,
				const float value) -> void {
	glUniform1f(glGetUniformLocation(shaderId, uniformName.c_str()), value);
}

auto setUniform(const unsigned int shaderId,
				const std::string& uniformName,
				const glm::mat4& value) -> void {
	glUniformMatrix4fv(glGetUniformLocation(shaderId, uniformName.c_str()), 1,
					   GL_FALSE, glm::value_ptr(value));
}

auto setUniform(const unsigned int shaderId,
				const std::string& uniformName,
				const float* value,
				const int size) -> void {
	glUniform1fv(glGetUniformLocation(shaderId, uniformName.c_str()), size,
				 value);
}
