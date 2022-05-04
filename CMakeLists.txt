cmake_minimum_required(VERSION 3.15)

find_package(Git QUIET REQUIRED)

set(APP_NAME "SICKBEATBETTY")
set(APP_VENDOR "HOWARDLOGIC")
set(APP_HUMAN_NAME "Sick Beat Betty")
set(APP_HUMAN_VENDOR "Howard Logic")
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
set(BUILD_INFO_NAME "${APP_HUMAN_NAME}")
set(BUILD_INFO_VERSION "${APP_VERSION}")
string(TIMESTAMP BUILD_INFO_DATE "%Y-%m-%d")
string(TIMESTAMP BUILD_INFO_TIME "%H:%M:%S")
cmake_host_system_information(RESULT BUILD_INFO_HOSTNAME QUERY HOSTNAME)
configure_file(src/buildinfo.cpp.in buildinfo.cpp)

project(${APP_NAME} VERSION ${APP_VERSION})

add_subdirectory(JUCE)                    # JUCE is a submodule.  Make sure it has been properly cloned.

juce_add_plugin(${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    COMPANY_NAME "${APP_HUMAN_VENDOR}"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE 
    COPY_PLUGIN_AFTER_BUILD TRUE 
    PLUGIN_MANUFACTURER_CODE HowL
    PLUGIN_CODE SBB1
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "${APP_HUMAN_NAME}"
    
)

include_directories(src)

target_sources(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/buildinfo.cpp
        src/plugineditor.cpp
        src/pluginprocessor.cpp
        src/beatgen.cpp
        src/beatgengroup.cpp
        src/beatgenui.cpp
        src/beatgenclockui.cpp
        src/paramslider.cpp
        src/parambutton.cpp
        src/paramcombobox.cpp
        src/beatvisualizer.cpp
        src/aboutui.cpp
        src/applogger.cpp
        src/paramhelper.cpp
)

juce_add_binary_data(IconBinaryData
    SOURCES
        icons/drum.png
        icons/dice.png 
        icons/reload.png
        icons/betty-white-devil-horns.png
)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        IconBinaryData
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

install(TARGETS ${PROJECT_NAME}_Standalone
    DESTINATION "."
    COMPONENT Standalone
)

install(TARGETS ${PROJECT_NAME}_VST3
    LIBRARY DESTINATION "."
    COMPONENT VST3
)
    
set(CPACK_GENERATOR NSIS)
set(CPACK_NSIS_VST3_INSTALL_DIRECTORY "$PROGRAMFILES64\\\\Common Files\\\\VST3\\\\${APP_HUMAN_NAME}.vst3")

#set(CPACK_GENERATOR WIX)
#set(CPACK_WIX_UPGRADE_GUID "6541AC2E-DA46-481C-9CF6-FE3CDF3A4C3F")

set(CPACK_COMPONENT_STANDALONE_DISPLAY_NAME "Standalone App")
set(CPACK_COMPONENT_STANDALONE_DESCRIPTION "Version of Sick Beat Betty that runs as a standalone application")

set(CPACK_COMPONENT_VST3_DISPLAY_NAME "VST3 Plugin")
set(CPACK_COMPONENT_VST3_DESCRIPTION "Version of Sick Beat Betty that runs as a VST3 plugin")

set(CPACK_COMPONENTS_ALL VST3 Standalone)
set(CPACK_COMPONENT_GROUPING ALL_COMPONENTS_IN_ONE)
set(CPACK_PACKAGE_NAME ${APP_HUMAN_NAME})
set(CPACK_PACKAGE_VENDOR ${APP_HUMAN_VENDOR})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${APP_HUMAN_NAME})
#set(CPACK_PACKAGE_DESCRIPTION_FILE "description.txt")
#set(CPACK_RESOURCE_FILE_WELCOME "welcome.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
SET(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/InstallerConfig.cmake")

INCLUDE(CPack)

#cpack_add_component(${PROJECT_NAME}_VST3
#    DISPLAY_NAME "VST 3 Plugin"
#)

#cpack_add_component(${PROJECT_NAME}_Standalone
#    DISPLAY_NAME "Standalone EXE"
#)

# Hack to allow building a installer from within VS Code
ADD_CUSTOM_TARGET(installer
    COMMAND "${CMAKE_CPACK_COMMAND}"
        "-C" "Debug"
        "--config" "${CMAKE_BINARY_DIR}/InstallerConfig.cmake"
    COMMENT "Running CPack. Please wait..."
    DEPENDS ${PROJECT_NAME}
)

