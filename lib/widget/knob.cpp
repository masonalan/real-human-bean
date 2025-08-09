//
// Created by James Pickering on 8/5/25.
//

#include "knob.hpp"

#include "../event.hpp"
#include "../graphics.hpp"
#include "../ui.hpp"

#include <juce_opengl/juce_opengl.h>
#include <glm/ext/matrix_transform.hpp>

using namespace ::juce::gl;

auto knobUpdate(Knob& knob, const Mouse& mouse) -> void {
	if (!mouse.isPressed) {
		knob.hovered = quadContainsPoint(knob.quad, mouse.pos);
	}

	if (mouse.events & EventMousePressed) {
		if (knob.hovered) {
			knob.rotating = true;
			knob.initRotation = knob.rotation;
		}
	} else if (mouse.events & EventMouseReleased) {
		knob.rotating = false;
	}

	if (knob.rotating) {
		knob.rotation =
			std::clamp(knob.initRotation -
						   glm::radians((mouse.pos.x - mouse.mouseDownPos.x)),
					   MaxKnobRotation, MinKnobRotation);
		knob.value = std::abs((knob.rotation - MinKnobRotation) /
							  (MinKnobRotation - MaxKnobRotation));
	}
}

auto knobRender(const Knob& knob, const GraphicsContext& graphics) -> void {
	auto quad = knob.quad;
	quad.size *= 2.f;
	auto model = quadToModel(quad);

	std::cout << "[*] Starting to draw knob" << std::endl;

	glUseProgram(graphics.circleShader.id);
	setUniform(graphics.circleShader.id, "model", model);
	setUniform(graphics.circleShader.id, "val", knob.value);

	std::cout << "[*] Using circle shader" << std::endl;

	glBindVertexArray(graphics.quadVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(graphics.shader.id);

	std::cout << "[*] Back to regular shader" << std::endl;

	model = knobApplyRotationToModel(quadToModel(knob.quad), knob.rotation);

	std::cout << "[*] Drawing knob" << std::endl;

	setUniform(graphics.shader.id, "model", model);
	setUniform(graphics.shader.id, "hovered", knob.hovered);
	setUniform(graphics.shader.id, "rotating", knob.rotating);
	setUniform(graphics.shader.id, "isImage", true);

	glBindTexture(GL_TEXTURE_2D, graphics.knobTexId);
	glBindVertexArray(graphics.quadVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	std::cout << "[*] Doneeeeee" << std::endl;
}

auto knobInitWithValue(Knob& knob, const float value) -> void {
	knob.value = value;
	knob.rotation = knobValueToRotation(value);
}

auto knobValueToRotation(const float value) -> float {
	return MinKnobRotation - value * (MinKnobRotation - MaxKnobRotation);
}

auto knobRotationToValue(const float rotation) -> float {
	return std::abs((rotation - MinKnobRotation) /
					(MinKnobRotation - MaxKnobRotation));
}

auto knobApplyRotationToModel(const glm::mat4& model, const float rotation)
	-> glm::mat4 {
	return glm::rotate(model, rotation, glm::vec3{0, 0, 1});
}
