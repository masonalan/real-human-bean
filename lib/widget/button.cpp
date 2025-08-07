//
// Created by James Pickering on 8/5/25.
//

#include "button.hpp"

#include "../event.hpp"
#include "../graphics.hpp"
#include "../mouse.hpp"

#include <juce_opengl/juce_opengl.h>

using namespace ::juce::gl;

auto buttonUpdate(Button& button, const Mouse& mouse) -> void {
	if (quadContainsPoint(button.quad, mouse.pos)) {
		if (mouse.isPressed) {
			if (!button.pressed) {
				button.events |= EventMousePressed;
				button.pressed = true;
			}
		} else {
			if (button.pressed) {
				button.events |= EventMouseReleased;
				button.pressed = false;
			}
		}
		button.hovered = true;
	} else {
		button.hovered = false;
	}
}

auto buttonRender(const Button& button, const GraphicsContext& graphics)
	-> void {
	const auto btnModel = quadToModel(button.quad);
	setUniform(graphics.shader.id, "model", btnModel);

	if (button.hovered) {
		if (button.pressed) {
			glBindTexture(GL_TEXTURE_2D, graphics.reseedButtonPressedTex);
		} else {
			glBindTexture(GL_TEXTURE_2D, graphics.reseedButtonHoveredTex);
		}
	} else {
		glBindTexture(GL_TEXTURE_2D, graphics.reseedButtonTex);
	}

	glBindVertexArray(graphics.quadVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
