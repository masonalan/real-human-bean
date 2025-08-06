//
// Created by James Pickering on 7/21/25.
//

#include "ui.hpp"

#include "config.hpp"
#include "engine.hpp"
#include "event.hpp"
#include "graphics.hpp"
#include "quad.hpp"
#include "texture.hpp"

#include <juce_opengl/juce_opengl.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

using namespace ::juce::gl;

inline auto valueHasChanged(const float value, const float prevValue) -> bool {
	return std::fabs(value - prevValue) > Epsilon;
}

inline auto applyStateToUi(Ui& ui,
						   const State& state,
						   const GraphicsContext& graphics) -> void {
	setTextureData(graphics.powerTexId, state.offsets.spectrum.data(),
				   state.offsets.spectrum.size());
	setTextureData(graphics.offsetGraphTexId, state.offsets.normOffsets.data(),
				   state.offsets.normOffsets.size());

	ui.cells.clear();
	for (auto i = 0; i < state.stepsI; ++i) {
		const auto cell =
			glm::vec2{i % OffsetDiagramCols, i / OffsetDiagramCols};
		const auto cellPos =
			OffsetDiagramTopLeft + (OffsetDiagramCellStep * cell);

		ui.cells.push_back(quadFromPsQuad(cellPos, BarSize));
	}
}

auto setupUi(Ui& ui) -> void {
	ui.knobAlpha.quad = quadFromPsQuad({375, KnobY}, KnobSize);
	ui.knobSteps.quad = quadFromPsQuad({594, KnobY}, KnobSize);
	ui.knobVariance.quad = quadFromPsQuad({815, KnobY}, KnobSize);
	ui.knobLookahead.quad = quadFromPsQuad({1029, KnobY}, KnobSize);
	ui.graphPowerSpectrum = quadFromPsQuad({36, 217}, GraphSize);
	ui.graphOffset = quadFromPsQuad({36, 447}, GraphSize);
	ui.diagramOffset = quadFromPsQuad({1256.57, 454.76}, {188.04, 21.9});
	ui.buttonReseed.quad = quadFromPsQuad({37, 655}, ReseedButtonSize);
}

auto updateUi(Ui& ui, State& state, const GraphicsContext& graphics) -> void {
	state.variance.store(ui.knobVariance.value);
	state.lookahead.store(ui.knobLookahead.value);

	if (ui.buttonReseed.events & EventMousePressed) {
		state.queuedSeedRecalc.store(true);
		ui.buttonReseed.events = 0;
	}

	if (valueHasChanged(state.alpha.load(), ui.knobAlpha.value) ||
		valueHasChanged(state.steps.load(), ui.knobSteps.value)) {
		state.alpha.store(ui.knobAlpha.value);
		state.steps.store(ui.knobSteps.value);
		state.queuedOffsetRecalc.store(true);
	}

	if (state.eventOffsetsUpdated.load() || ui.isFresh) {
		applyStateToUi(ui, state, graphics);
		state.eventOffsetsUpdated.store(false);
	}

	knobUpdate(ui.knobAlpha, ui.mouse);
	knobUpdate(ui.knobSteps, ui.mouse);
	knobUpdate(ui.knobVariance, ui.mouse);
	knobUpdate(ui.knobLookahead, ui.mouse);
}

auto renderUi(Ui& ui, const State& state, const GraphicsContext& graphics)
	-> void {
	if (ui.isFresh) {
		glEnable(GL_BLEND);
		setUniform(graphics.shader.id, "tex0", 0);
		setUniform(graphics.shader.id, "enableSaturation", false);

		ui.isFresh = false;
	}

	// For some dumb reason this is being unset every damn loop (juce u succ)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(1.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto backgroundQuad = glm::mat4{1};
	backgroundQuad = glm::translate(backgroundQuad, {0, 0, 0});
	backgroundQuad = glm::scale(backgroundQuad, {2, 2, 0});

	glUseProgram(graphics.shader.id);
	setUniform(graphics.shader.id, "model", backgroundQuad);
	setUniform(graphics.shader.id, "isImage", true);

	glActiveTexture(GL_TEXTURE0);
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

	if (quadContainsPoint(ui.buttonReseed.quad, ui.mouse.pos)) {
		if (ui.mouse.isPressed) {
			glBindTexture(GL_TEXTURE_2D, graphics.reseedButtonPressedTex);
			if (!ui.buttonReseed.pressed) {
				ui.buttonReseed.events |= EventMousePressed;
				ui.buttonReseed.pressed = true;
			}
		} else {
			if (ui.buttonReseed.pressed) {
				ui.buttonReseed.events |= EventMouseReleased;
				ui.buttonReseed.pressed = false;
			}
			glBindTexture(GL_TEXTURE_2D, graphics.reseedButtonHoveredTex);
		}
	} else {
		glBindTexture(GL_TEXTURE_2D, graphics.reseedButtonTex);
	}

	glBindVertexArray(graphics.quadVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// TODO: figure out why this is here
	auto fig3Quad = ui.diagramOffset;
	fig3Quad.pos.y = ui.cells.back().pos.y - 35.f;
	setUniform(graphics.shader.id, "model", createModelQuad(fig3Quad));

	glBindTexture(GL_TEXTURE_2D, graphics.fig3TexId);
	glBindVertexArray(graphics.quadVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, graphics.notesFbo);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, ui.windowSize.x * 2, ui.windowSize.y * 2);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto i = 0;

	for (auto& pair : ui.cells) {
		auto model = createModelQuad(pair);
		setUniform(graphics.shader.id, "model", model);

		glBindTexture(GL_TEXTURE_2D, graphics.normalTexId);
		glBindVertexArray(graphics.quadVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		auto offsetQuad = pair;
		offsetQuad.pos.x += getOffsetAt(state, i) / 100.f;

		++i;

		model = createModelQuad(offsetQuad);
		setUniform(graphics.shader.id, "model", model);

		glBindTexture(GL_TEXTURE_2D, graphics.shiftTexId);
		glBindVertexArray(graphics.quadVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBlendFunc(GL_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, ui.windowSize.x * 2, ui.windowSize.y * 2);

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
