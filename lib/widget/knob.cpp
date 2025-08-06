//
// Created by James Pickering on 8/5/25.
//

#include "knob.hpp"

#include "../ui.hpp"
#include "../event.hpp"

#include <algorithm>

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
