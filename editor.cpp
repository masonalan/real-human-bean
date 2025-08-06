//
// Created by James Pickering on 3/26/25.
//

#include "editor.hpp"

#include "lib/engine.hpp"
#include "lib/event.hpp"
#include "lib/graphics.hpp"
#include "lib/ui.hpp"

#include <glm/ext/matrix_transform.hpp>

using namespace ::juce::gl;

Editor::Editor(Processor& processor)
	: AudioProcessorEditor{&processor}, _processor{processor} {
	openGLComponent = new OpenGLComponent{processor.ctx};
	openGLComponent->setOpaque(false);

	addAndMakeVisible(openGLComponent);
	setSize(config::WindowSize.x, config::WindowSize.y);
}

Editor::~Editor() {
	delete openGLComponent;
};

auto Editor::paint(juce::Graphics& g) -> void {
	g.fillAll(juce::Colours::rebeccapurple);

	g.setColour(juce::Colours::white);
	g.setFont(15.f);
	g.drawFittedText("Hello World!", getLocalBounds(),
					 juce::Justification::centred, 1);
}

auto Editor::resized() -> void {
	openGLComponent->setBounds(getLocalBounds());
}

OpenGLComponent::OpenGLComponent(State& state) : state{state} {
	setSize(config::WindowSize.x, config::WindowSize.y);
	openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::openGL4_1);
}

OpenGLComponent::~OpenGLComponent() {
	shutdownOpenGL();
}

auto OpenGLComponent::initialise() -> void {
	setupGraphics(graphics);
	setupUi(ui);

	knobInitWithValue(ui.knobAlpha, state.alpha.load());
	knobInitWithValue(ui.knobSteps, state.steps.load());
	knobInitWithValue(ui.knobVariance, state.variance.load());
	knobInitWithValue(ui.knobLookahead, state.lookahead.load());
}

auto OpenGLComponent::shutdown() -> void {}

auto OpenGLComponent::render() -> void {
	ui.windowSize = {getWidth(), getHeight()};

	const auto mousePosRel = getMouseXYRelative();
	ui.mouse.pos = glm::vec2{mousePosRel.x, mousePosRel.y} -
				   glm::vec2{config::WindowSize.x, config::WindowSize.y} / 2.f;
	ui.mouse.pos.y = -ui.mouse.pos.y;

	ui.mouse.events = 0;
	if (const auto isPressed = isMouseButtonDown();
		isPressed && !ui.mouse.isPressed) {
		ui.mouse.isPressed = true;
		ui.mouse.events |= EventMousePressed;
		ui.mouse.mouseDownPos = ui.mouse.pos;
	} else if (!isPressed && ui.mouse.isPressed) {
		ui.mouse.isPressed = false;
		ui.mouse.events |= EventMouseReleased;
	}

	updateUi(ui, state, graphics);
	renderUi(ui, state, graphics);
}