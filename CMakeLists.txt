cmake_minimum_required(VERSION 3.15)
project(SICK_BEAT_BETTY VERSION 0.0.1)

add_subdirectory(JUCE)                    # JUCE is a submodule.  Make sure it has been properly cloned.

juce_add_plugin(SickBeatBetty
    VERSION 1.0.0
    COMPANY_NAME "Howard Logic"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE 
    COPY_PLUGIN_AFTER_BUILD TRUE 
    PLUGIN_MANUFACTURER_CODE HowL
    PLUGIN_CODE SBB1
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Sick Beat Betty"
    
)

target_sources(SickBeatBetty
    PRIVATE
        src/plugineditor.cpp
        src/pluginprocessor.cpp
        src/beatgen.cpp
        src/beatgenui.cpp
        src/beatgenclockui.cpp
        src/paramslider.cpp
        src/parambutton.cpp
)

juce_add_binary_data(IconBinaryData
    SOURCES
        icons/drum.png
)

target_compile_definitions(SickBeatBetty
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(SickBeatBetty
    PRIVATE
        IconBinaryData
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)


