//
// Created by James Pickering on 3/26/25.
//

#pragma once

#include "lib/controls.hpp"
#include "lib/engine.hpp"
#include "lib/graphics.hpp"
#include "processor.hpp"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_opengl/juce_opengl.h>
#include <glm/glm.hpp>

class OpenGLComponent;

class Editor : public juce::AudioProcessorEditor {
   public:
	explicit Editor(Processor& processor);
	~Editor() override;

	auto paint(juce::Graphics& g) -> void override;
	auto resized() -> void override;

	Processor& _processor;
	OpenGLComponent* openGLComponent;

   private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
};

class OpenGLComponent final : public juce::OpenGLAppComponent {
   public:
	explicit OpenGLComponent(State& state);
	~OpenGLComponent() override;

	auto shutdown() -> void override;
	auto render() -> void override;
	auto initialise() -> void override;

	GraphicsContext graphics;
	State& state;
	Ui ui;
};