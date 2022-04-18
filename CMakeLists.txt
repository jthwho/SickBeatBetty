cmake_minimum_required(VERSION 3.15)

find_package(Git QUIET REQUIRED)

set(APP_NAME "Sick Beat Betty")
set(APP_VERSION "1.0.0")

execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RESULT_VARIABLE res
    OUTPUT_VARIABLE BUILD_INFO_REPOIDENT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set_property(GLOBAL APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/.git/index"
)

# Capture any other build info variables
set(BUILD_INFO_NAME ${APP_NAME})
set(BUILD_INFO_VERSION ${APP_VERSION})
string(TIMESTAMP BUILD_INFO_DATE "%Y-%m-%d")
string(TIMESTAMP BUILD_INFO_TIME "%H:%M:%S")
cmake_host_system_information(RESULT BUILD_INFO_HOSTNAME QUERY HOSTNAME)
configure_file(src/buildinfo.cpp.in buildinfo.cpp)

project(SICK_BEAT_BETTY VERSION ${APP_VERSION})

add_subdirectory(JUCE)                    # JUCE is a submodule.  Make sure it has been properly cloned.

juce_add_plugin(SickBeatBetty
    VERSION ${PROJECT_VERSION}
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
    PRODUCT_NAME ${APP_NAME}
    
)

include_directories(src)

target_sources(SickBeatBetty
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/buildinfo.cpp
        src/plugineditor.cpp
        src/pluginprocessor.cpp
        src/beatgen.cpp
        src/beatgenui.cpp
        src/beatgenclockui.cpp
        src/paramslider.cpp
        src/parambutton.cpp
        src/beatvisualizer.cpp
        src/aboutui.cpp
)

juce_add_binary_data(IconBinaryData
    SOURCES
        icons/drum.png
        icons/dice.png 
        icons/reload.png
        icons/betty-white-devil-horns.jpg
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

