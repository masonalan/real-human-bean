//
// Created by James Pickering on 7/21/25.
//

#include "controls.hpp"

#include "engine.hpp"
#include "graphics.hpp"
#include "quad.hpp"
#include "texture.hpp"

#include <juce_opengl/juce_opengl.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

using namespace ::juce::gl;

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

auto hasEvent(const int events, const int event) -> bool {
	return events & event;
}

auto clearEvent(int& events, const int event) -> void {
	events &= ~event;
}

auto setupUi(Ui& ui) -> void {
	ui.knobAlpha.quad = fromPsQuad({375, KnobY}, KnobSize);
	ui.knobSteps.quad = fromPsQuad({594, KnobY}, KnobSize);
	ui.knobVariance.quad = fromPsQuad({815, KnobY}, KnobSize);
	ui.knobLookahead.quad = fromPsQuad({1029, KnobY}, KnobSize);
	ui.graphPowerSpectrum = fromPsQuad({36, 217}, GraphSize);
	ui.graphOffset = fromPsQuad({36, 447}, GraphSize);
	ui.diagramOffset = fromPsQuad({1256.57, 454.76}, {188.04, 21.9});
	ui.buttonReseed.quad = fromPsQuad({37, 655}, ReseedButtonSize);
}

auto updateUi(Ui& ui, State& state, const GraphicsContext& graphics) -> void {
	if (std::fabs(state.alpha.load() - ui.knobAlpha.value) > Epsilon) {
		state.alpha.store(ui.knobAlpha.value);
		state.alphaChanged = true;
	}

	if (std::fabs(state.steps - ui.knobSteps.value) > Epsilon) {
		state.steps = ui.knobSteps.value;
		state.stepsI = stepsFromKnobValue(state.steps);
		state.stepsChanged = true;
	}

	state.variance = ui.knobVariance.value;
	state.lookahead = ui.knobLookahead.value;

	if (state.stepsChanged || state.alphaChanged || ui.isFresh) {
		constexpr auto startPos = glm::vec2{1240, 217};
		ui.shiftPairs.clear();
		for (auto i = 0; i < state.stepsI; ++i) {
			auto col = i % 3;
			auto row = i / 3;

			auto pos = startPos;
			pos.x += 89.f * col;
			pos.y += 39.f * row;

			auto pair = ShiftPair{};
			pair.onBeat.size = fromPsSize(BarSize);
			pair.onBeat.pos = fromPsPos(pos, BarSize);
			pair.shifted.size = fromPsSize(BarSize);
			pair.shifted.pos = fromPsPos(pos, BarSize);
			ui.shiftPairs.push_back(pair);
		}
	}

	if (ui.buttonReseed.events & MousePressEvent) {
		recalcRandVals();
		ui.buttonReseed.events = 0;
		state.alphaChanged = true;
	}

	if (state.alphaChanged || state.stepsChanged || ui.isFresh) {
		applyState(state);
		// const auto ps = genFractalOffsets(state.stepsI,
		// 								  state.alpha.load() * 0.3f + 0.5f, 10);
		// state.minOffset = ps.minOffset;
		// auto floatArr = std::vector<float>{};
		// for (auto val : ps.spectrum) {
		// 	floatArr.push_back(val);
		// }
		setTextureData(graphics.powerTexId, state.offsets.spectrum.data(),
					   state.offsets.spectrum.size());
		setTextureData(graphics.offsetGraphTexId,
					   state.offsets.normOffsets.data(),
					   state.offsets.normOffsets.size());

		state.alphaChanged = false;
		state.stepsChanged = false;

		ui.isFresh = false;
	}

	auto updateKnob = [&ui](auto& knob) {
		if (!ui.mouse.isPressed) {
			knob.hovered = containsPoint(knob.quad, ui.mouse.pos);
		}

		if (ui.mouse.events & MousePressEvent) {
			if (knob.hovered) {
				knob.rotating = true;
				knob.initRotation = knob.rotation;
			}
		} else if (ui.mouse.events & MouseReleaseEvent) {
			knob.rotating = false;
		}

		if (knob.rotating) {
			knob.rotation = std::clamp(
				knob.initRotation -
					glm::radians((ui.mouse.pos.x - ui.mouse.mouseDownPos.x)),
				MaxKnobRotation, MinKnobRotation);
			knob.value = std::abs((knob.rotation - MinKnobRotation) /
								  (MinKnobRotation - MaxKnobRotation));
		}
	};

	updateKnob(ui.knobAlpha);
	updateKnob(ui.knobSteps);
	updateKnob(ui.knobVariance);
	updateKnob(ui.knobLookahead);
}

auto renderUi(Ui& ui, const State& state, const GraphicsContext& graphics)
	-> void {
	glUseProgram(graphics.shader.id);
	glActiveTexture(GL_TEXTURE0);
	setUniform(graphics.shader.id, "isNoise", false);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(1.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto m = glm::mat4{1};
	m = glm::translate(m, glm::vec3{0, 0, 0});
	m = glm::scale(m, glm::vec3{2, 2, 0});

	setUniform(graphics.shader.id, "model", m);
	setUniform(graphics.shader.id, "hovered", false);
	setUniform(graphics.shader.id, "rotating", false);
	setUniform(graphics.shader.id, "isImage", true);
	setUniform(graphics.shader.id, "tex0", 0);
	setUniform(graphics.shader.id, "enableSaturation", false);

	glBindTexture(GL_TEXTURE_2D, graphics.bgTextureId);
	glBindVertexArray(graphics.quadVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	auto createModelQuad = [](const Quad& knob) {
		auto model = glm::mat4{1.f};
		model = glm::translate(
			model, glm::vec3{knob.pos / config::HalfWindowSize, 0.1f});
		model = glm::scale(model,
						   glm::vec3{knob.size / config::HalfWindowSize, 1.f});
		return model;
	};

	auto createModel = [&createModelQuad](const Knob& knob) {
		auto model = createModelQuad(knob.quad);
		model = glm::rotate(model, knob.rotation, glm::vec3{0, 0, 1});
		return model;
	};

	auto renderKnob = [&createModel, createModelQuad,
					   graphics](const Knob& knob) {
		auto quad = knob.quad;
		quad.size *= 2.f;
		auto model = createModelQuad(quad);

		glUseProgram(graphics.circleShader.id);
		setUniform(graphics.circleShader.id, "model", model);
		setUniform(graphics.circleShader.id, "val", knob.value);

		glBindVertexArray(graphics.quadVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glUseProgram(graphics.shader.id);

		model = createModel(knob);

		setUniform(graphics.shader.id, "model", model);
		setUniform(graphics.shader.id, "hovered", knob.hovered);
		setUniform(graphics.shader.id, "rotating", knob.rotating);
		setUniform(graphics.shader.id, "isImage", true);

		glBindTexture(GL_TEXTURE_2D, graphics.knobTexId);
		glBindVertexArray(graphics.quadVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	};

	renderKnob(ui.knobAlpha);
	renderKnob(ui.knobSteps);
	renderKnob(ui.knobVariance);
	renderKnob(ui.knobLookahead);

	auto btnModel = createModelQuad(ui.buttonReseed.quad);
	setUniform(graphics.shader.id, "model", btnModel);

	if (containsPoint(ui.buttonReseed.quad, ui.mouse.pos)) {
		if (ui.mouse.isPressed) {
			glBindTexture(GL_TEXTURE_2D, graphics.reseedButtonPressedTex);
			if (!ui.buttonReseed.pressed) {
				ui.buttonReseed.events |= MousePressEvent;
				ui.buttonReseed.pressed = true;
			}
		} else {
			if (ui.buttonReseed.pressed) {
				ui.buttonReseed.events |= MouseReleaseEvent;
				ui.buttonReseed.pressed = false;
			}
			glBindTexture(GL_TEXTURE_2D, graphics.reseedButtonHoveredTex);
		}
	} else {
		glBindTexture(GL_TEXTURE_2D, graphics.reseedButtonTex);
	}

	glBindVertexArray(graphics.quadVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	if (!ui.shiftPairs.empty()) {
		auto fig3Quad = ui.diagramOffset;
		fig3Quad.pos.y = ui.shiftPairs.back().onBeat.pos.y - 35.f;
		setUniform(graphics.shader.id, "model", createModelQuad(fig3Quad));

		glBindTexture(GL_TEXTURE_2D, graphics.fig3TexId);
		glBindVertexArray(graphics.quadVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, graphics.notesFbo);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, ui.size.x * 2, ui.size.y * 2);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto i = 0;
	for (auto& pair : ui.shiftPairs) {
		auto model = createModelQuad(pair.onBeat);
		setUniform(graphics.shader.id, "model", model);

		glBindTexture(GL_TEXTURE_2D, graphics.normalTexId);
		glBindVertexArray(graphics.quadVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// const auto val = ctx.vals[i];
		pair.shifted.pos.x = pair.onBeat.pos.x + getOffsetAt(state, i) / 100.f;

		++i;

		model = createModelQuad(pair.shifted);
		setUniform(graphics.shader.id, "model", model);

		glBindTexture(GL_TEXTURE_2D, graphics.shiftTexId);
		glBindVertexArray(graphics.quadVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBlendFunc(GL_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, ui.size.x * 2, ui.size.y * 2);

	auto quad = Quad{};
	quad.pos = {0, 0};
	quad.size = config::WindowSize;

	auto model = createModelQuad(quad);
	setUniform(graphics.shader.id, "model", model);
	setUniform(graphics.shader.id, "enableSaturation", true);

	glBindTexture(GL_TEXTURE_2D, graphics.notesTex);
	glBindVertexArray(graphics.quadVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	setUniform(graphics.shader.id, "enableSaturation", false);

	glUseProgram(graphics.graphShader.id);

	model = createModelQuad(ui.graphPowerSpectrum);
	setUniform(graphics.graphShader.id, "model", model);
	setUniform(graphics.graphShader.id, "powerTex", 1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, graphics.powerTexId);

	glBindVertexArray(graphics.quadVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	model = createModelQuad(ui.graphOffset);
	setUniform(graphics.graphShader.id, "model", model);
	setUniform(graphics.graphShader.id, "powerTex", 1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, graphics.offsetGraphTexId);

	glBindVertexArray(graphics.quadVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(graphics.shader.id);
}
