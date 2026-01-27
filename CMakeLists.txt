cmake_minimum_required(VERSION 3.15)

project(MidiFartSniffer VERSION 1.0.0)

# Add JUCE as a subdirectory
# This will fetch JUCE from GitHub if not already available
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
)
FetchContent_MakeAvailable(JUCE)

# Add the plugin target
juce_add_plugin(MidiFartSniffer
    COMPANY_NAME "Tri$oft"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    PLUGIN_MANUFACTURER_CODE Trif
    PLUGIN_CODE MiFS
    FORMATS VST3
    PRODUCT_NAME "MidiFartSniffer"
)

# Add source files
target_sources(MidiFartSniffer
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

# Include directories
target_include_directories(MidiFartSniffer
    PRIVATE
        Source
)

# Link JUCE libraries
target_link_libraries(MidiFartSniffer
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Add compile definitions
target_compile_definitions(MidiFartSniffer
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
