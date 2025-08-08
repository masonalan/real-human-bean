cmake_minimum_required(VERSION 3.30)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Minimum OS X deployment version" FORCE)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
project(real_human_bean VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)

find_package(JUCE CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)

add_subdirectory(test)

juce_add_binary_data(Resources
        SOURCES
        resources/image/bg3.png
        resources/image/btn-play.png
        resources/image/btn-play-hovered.png
        resources/image/btn-play-pressed.png
        resources/image/fig3.png
        resources/image/knob2.png
        resources/image/normal.png
        resources/image/shift.png
        resources/shader/shader.vert
        resources/shader/shader.frag
        resources/shader/circle-shader.frag
        resources/shader/graph-shader.frag
        resources/shader/noise-shader.frag
)

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

target_sources(real_human_bean
        PRIVATE
        editor.cpp
        processor.cpp
        lib/texture.cpp
        lib/graphics.cpp
        lib/ui.cpp
        lib/engine.cpp
        lib/serialize.cpp
        lib/quad.cpp
        lib/widget/knob.cpp
        lib/widget/button.cpp)


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
        juce::juce_recommended_warning_flags
        Resources)
target_link_libraries(real_human_bean PRIVATE glm::glm)
target_include_directories(real_human_bean PRIVATE ${Stb_INCLUDE_DIR})

set(CODESIGN_ID "Developer ID Application: James Pickering (TEAMID)")  # macOS
set(NOTARIZE_APPLE_ID "you@example.com")                         # macOS
set(NOTARIZE_TEAM_ID "TEAMID")                                   # macOS
set(NOTARIZE_PASSWORD "app-specific-password")                   # macOS

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(APPLE)
        add_custom_command(TARGET real_human_bean
                POST_BUILD
                COMMAND codesign --deep --force --options runtime
                --sign "${CODESIGN_ID}"
                "${VST3_PATH}"
                COMMAND xcrun notarytool submit "${VST3_PATH}"
                --apple-id "${NOTARIZE_APPLE_ID}"
                --team-id "${NOTARIZE_TEAM_ID}"
                --password "${NOTARIZE_PASSWORD}"
                --wait
                COMMAND xcrun stapler staple "${VST3_PATH}"
                COMMENT "Signing and notarizing VST3 for macOS"
        )
    endif()
endif()
