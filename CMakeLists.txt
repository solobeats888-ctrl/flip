cmake_minimum_required(VERSION 3.22)

# Universal binary (Apple Silicon + Intel). Must be set before project().
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")

project(Flipper VERSION 0.9.3 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

option(FLIPPER_BUILD_SNAPSHOT "Build the headless UI snapshot tool" OFF)

# ---------------------------------------------------------------- JUCE
# -DJUCE_PATH=/ruta/a/JUCE para usar una copia local; si no, se baja de GitHub.
if(JUCE_PATH)
    add_subdirectory(${JUCE_PATH} JUCE EXCLUDE_FROM_ALL)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.8
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

# ---------------------------------------------------------------- assets
juce_add_binary_data(FlipperAssets
    NAMESPACE FlipperBinary
    SOURCES
        Assets/Background/main_playfield.jpg
        Assets/Controls/knob_macro_1.png
        Assets/Controls/knob_macro_2.png
        Assets/Controls/knob_macro_3.png
        Assets/Controls/knob_macro_4.png
        Assets/Controls/knob_macro_5.png
        Assets/Controls/knob_macro_6.png
        Assets/Controls/knob_macro_7.png
        Assets/Controls/knob_macro_8.png
        Assets/Controls/knob_character.png
        Assets/Controls/mode_led_on.png
        Assets/Controls/mode_led_off.png
        Assets/Characters/minimoog_character.png
        Assets/Characters/prophet5_character.png
        Assets/Characters/jupiter8_character.png
        Assets/Characters/juno_character.png
        Assets/Characters/cs80_character.png
        Assets/Characters/dx7_character.png
        Assets/Characters/m1_character.png
        Assets/Characters/arp2600_character.png
        Assets/Characters/vcs3_character.png
        Assets/effects/ball/ball_chrome.png
        Assets/objects/pill_red.png
        Assets/objects/pill_purple.png
        Assets/objects/pill_orange.png
        Assets/objects/pill_blue.png
        Assets/objects/coin_a.png
        Assets/objects/coin_b.png
        Assets/objects/cash.png
        Assets/objects/vinyl.png
        Assets/objects/bone_b.png
        Assets/objects/heart.png
        Assets/effects/sparks/burst_orange.png
        Assets/effects/sparks/star_a.png
        Assets/effects/sparks/star_b.png
        Assets/effects/sparks/burst_big.png
        Assets/effects/sparks/star_gold.png
        Assets/effects/sparks/sparkle_magenta.png
        Assets/effects/sparks/explosion.png
        Assets/effects/sparks/fire_burst.png
        Assets/effects/lightning/bolt_a.png
        Assets/effects/lightning/bolt_b.png
        Assets/effects/smoke/puff_a.png
        Assets/effects/smoke/puff_b.png
        Assets/effects/smoke/puff_c.png
        Assets/effects/smoke/wisp.png
        Assets/effects/smoke/swirl.png
        Assets/effects/rainbow/arc_wide.png
        Assets/effects/rainbow/arc_small.png
        Assets/Fonts/BebasNeue-Regular.ttf)
set_target_properties(FlipperAssets PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(FLIPPER_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Core/Parameters.cpp
    Source/Core/SoundGenerator.cpp
    Source/Core/PresetManager.cpp
    Source/Core/FactoryPresets.cpp
    Source/Core/PinballSfx.cpp
    Source/Anim/AnimationLayer.cpp
    Source/DSP/MinimoogEngine.cpp
    Source/DSP/Engine.cpp
    Source/DSP/EnginesAnalog.cpp
    Source/DSP/EnginesDigital.cpp
    Source/DSP/EnginesModular.cpp
    Source/DSP/SynthCore.cpp
    Source/UI/Theme.cpp
    Source/UI/Playfield.cpp
    Source/UI/Controls.cpp
    Source/UI/CharacterPanel.cpp
    Source/UI/LabPanel.cpp
    Source/UI/DesignPanel.cpp
    Source/UI/VintageControls.cpp
    Source/UI/DesignWidgets.cpp
    Source/Help/I18n.cpp
    Source/Help/Translations.cpp
    Source/Help/Help.cpp
    Source/Help/HelpUI.cpp
    Source/Core/Modulation.cpp
    Source/UI/ModUI.cpp
    Source/Teach/Lessons.cpp
    Source/Teach/TeacherUI.cpp)

# ---------------------------------------------------------------- plugin
set(FLIPPER_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND FLIPPER_FORMATS AU)
endif()

if(APPLE AND NOT DEFINED ENV{CI})
    set(FLIPPER_COPY_AFTER_BUILD TRUE)
else()
    set(FLIPPER_COPY_AFTER_BUILD FALSE)
endif()

juce_add_plugin(Flipper
    COMPANY_NAME                "Viszeversza"
    BUNDLE_ID                   "com.viszeversza.flipper"
    PRODUCT_NAME                "FLIPPER"
    PLUGIN_NAME                 "FLIPPER"
    DESCRIPTION                 "Vintage Synthesis Engine"
    VERSION                     ${PROJECT_VERSION}
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     ${FLIPPER_COPY_AFTER_BUILD}
    PLUGIN_MANUFACTURER_CODE    Vszv
    PLUGIN_CODE                 Flpr
    FORMATS                     ${FLIPPER_FORMATS}
    AU_MAIN_TYPE                kAudioUnitType_MusicDevice
    VST3_CATEGORIES             Instrument Synth)

target_sources(Flipper PRIVATE ${FLIPPER_SOURCES})
if(MSVC)
    target_compile_options(Flipper PRIVATE /utf-8)   # los textos en castellano estan en UTF-8
endif()
target_include_directories(Flipper PRIVATE Source)
target_compile_definitions(Flipper PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(Flipper
    PRIVATE
        FlipperAssets
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# ---------------------------------------------------------------- snapshot
# Herramienta para renderizar la interfaz a PNG sin abrir un DAW (revision visual).
if(FLIPPER_BUILD_SNAPSHOT)
    juce_add_console_app(FlipperSnapshot PRODUCT_NAME "FlipperSnapshot")
    target_sources(FlipperSnapshot PRIVATE tools/snapshot/Snapshot.cpp ${FLIPPER_SOURCES})
    target_include_directories(FlipperSnapshot PRIVATE Source)
    target_compile_definitions(FlipperSnapshot PRIVATE
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_STANDALONE_APPLICATION=1
        FLIPPER_SNAPSHOT=1)
    target_link_libraries(FlipperSnapshot PRIVATE
        FlipperAssets juce::juce_audio_utils juce::juce_dsp
        juce::juce_recommended_config_flags)

    juce_add_console_app(FlipperTests PRODUCT_NAME "FlipperTests")
    target_sources(FlipperTests PRIVATE tools/tests/Tests.cpp ${FLIPPER_SOURCES})
    target_include_directories(FlipperTests PRIVATE Source)
    target_compile_definitions(FlipperTests PRIVATE
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_STANDALONE_APPLICATION=1 FLIPPER_SNAPSHOT=1)
    target_link_libraries(FlipperTests PRIVATE
        FlipperAssets juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
