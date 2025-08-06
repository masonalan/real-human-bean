cmake_minimum_required(VERSION 3.30)
project(real_human_bean VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)

find_package(JUCE CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)

add_subdirectory(test)

juce_add_plugin(${PROJECT_NAME}
        COMPANY_NAME tyos
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        JUCE_VST3_CAN_REPLACE_VST2 FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE TYOS
        PLUGIN_CODE TYOS
        FORMATS VST3 AU Standalone
        PRODUCT_NAME "real human bean"
)

#source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

target_sources(real_human_bean
        PRIVATE
        editor.cpp
        processor.cpp
        lib/texture.cpp
        lib/noise.cpp
        lib/graphics.cpp
        lib/controls.cpp
        lib/engine.cpp
        lib/serialize.cpp
        lib/quad.cpp)


target_compile_definitions(real_human_bean
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(real_human_bean PRIVATE
        juce::juce_core
        juce::juce_analytics
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_data_structures
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_opengl
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_events
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
target_link_libraries(real_human_bean PRIVATE glm::glm)
target_include_directories(real_human_bean PRIVATE ${Stb_INCLUDE_DIR})