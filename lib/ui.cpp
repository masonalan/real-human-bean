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
#include "widget/button.hpp"

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
	ui.labelKnobDesc = quadFromPsQuad({336, 423}, {826, 62});
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

	buttonUpdate(ui.buttonReseed, ui.mouse);

	if (ui.knobAlpha.hovered) {
		ui.currDescTex = graphics.descAlpha;
	} else if (ui.knobSteps.hovered) {
		ui.currDescTex = graphics.descSteps;
	} else if (ui.knobVariance.hovered) {
		ui.currDescTex = graphics.descVariance;
	} else if (ui.knobLookahead.hovered) {
		ui.currDescTex = graphics.descLookahead;
	} else {
		ui.currDescTex = graphics.descNone;
	}
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

	knobRender(ui.knobAlpha, graphics);
	knobRender(ui.knobSteps, graphics);
	knobRender(ui.knobVariance, graphics);
	knobRender(ui.knobLookahead, graphics);

	buttonRender(ui.buttonReseed, graphics);

	if (ui.cells.size() > 0) {
		auto labelFig3Quad = ui.diagramOffset;
		labelFig3Quad.pos.y = ui.cells.back().pos.y - 35.f;
		setUniform(graphics.shader.id, "model", quadToModel(labelFig3Quad));

		glBindTexture(GL_TEXTURE_2D, graphics.labelFig3Tex);
		glBindVertexArray(graphics.quadVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	setUniform(graphics.shader.id, "model", quadToModel(ui.labelKnobDesc));

	glBindTexture(GL_TEXTURE_2D, ui.currDescTex);
	glBindVertexArray(graphics.quadVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, graphics.notesFbo);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, ui.windowSize.x, ui.windowSize.y);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto i = 0;
	for (auto& pair : ui.cells) {
		auto model = quadToModel(pair);
		setUniform(graphics.shader.id, "model", model);

		glBindTexture(GL_TEXTURE_2D, graphics.normalTexId);
		glBindVertexArray(graphics.quadVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		auto offsetQuad = pair;
		offsetQuad.pos.x += getOffsetAt(state, i) / 100.f;

		model = quadToModel(offsetQuad);
		setUniform(graphics.shader.id, "model", model);

		glBindTexture(GL_TEXTURE_2D, graphics.shiftTexId);
		glBindVertexArray(graphics.quadVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		++i;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBlendFunc(GL_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, ui.windowSize.x, ui.windowSize.y);

	auto quadDiagram = Quad{};
	quadDiagram.pos = {0, 0};
	quadDiagram.size = config::WindowSize;

	auto model = quadToModel(quadDiagram);
	setUniform(graphics.shader.id, "model", model);
	setUniform(graphics.shader.id, "enableSaturation", true);

	glBindTexture(GL_TEXTURE_2D, graphics.notesTex);
	glBindVertexArray(graphics.quadVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	setUniform(graphics.shader.id, "enableSaturation", false);

	glUseProgram(graphics.graphShader.id);

	model = quadToModel(ui.graphPowerSpectrum);
	setUniform(graphics.graphShader.id, "model", model);
	setUniform(graphics.graphShader.id, "powerTex", 1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, graphics.powerTexId);
	glBindVertexArray(graphics.quadVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	model = quadToModel(ui.graphOffset);
	setUniform(graphics.graphShader.id, "model", model);
	setUniform(graphics.graphShader.id, "powerTex", 1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, graphics.offsetGraphTexId);
	glBindVertexArray(graphics.quadVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
