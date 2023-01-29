cmake_minimum_required(VERSION 3.15)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/")

find_package(Git QUIET REQUIRED)

set(APP_NAME "SICKBEATBETTY")
set(APP_VENDOR "HOWARDLOGIC")
set(APP_HUMAN_NAME "Sick Beat Betty")
set(APP_HUMAN_VENDOR "Howard Logic")
set(APP_VERSION "1.0.4")
set(APP_VERSION_RC 0) # If this is a release candidate, set this value to the release candidate version (non zero)
set(APP_VERSION_BETA 1) # If this is a beta, set this value to the beta version (non-zero)


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

project(${APP_NAME} VERSION ${APP_VERSION})
add_subdirectory(JUCE)                    # JUCE is a submodule.  Make sure it has been properly cloned.

# Capture any other build info variables
# Must be below project()
if(NOT ${APP_VERSION_RC} EQUAL "0") 
    set(BUILD_INFO_VERSION "${APP_VERSION}-rc${APP_VERSION_RC}")
elseif(NOT ${APP_VERSION_BETA} EQUAL "0")
    set(BUILD_INFO_VERSION "${APP_VERSION}-beta${APP_VERSION_BETA}")
else()
    set(BUILD_INFO_VERSION "${APP_VERSION}")
endif()
set(BUILD_INFO_VERSION_BETA "${APP_VERSION_BETA}")
set(BUILD_INFO_VERSION_RC "${APP_VERSION_RC}")
set(BUILD_INFO_NAME "${APP_HUMAN_NAME}")
string(TIMESTAMP BUILD_INFO_DATE "%Y-%m-%d")
string(TIMESTAMP BUILD_INFO_TIME "%H:%M:%S")
set(BUILD_INFO_TYPE "${CMAKE_BUILD_TYPE}")
cmake_host_system_information(RESULT BUILD_INFO_HOSTNAME QUERY HOSTNAME)
configure_file(src/buildinfo.cpp.in buildinfo.cpp)

# Make sure we regenerate the buildinfo.cpp file each time we build.
add_custom_target(
    invalidate_build_info 
    ALL
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_SOURCE_DIR}/src/buildinfo.cpp.in
)

# FIXME: We'll want to do something better later.
set(app_icon "${CMAKE_SOURCE_DIR}/icons/betty-white-devil-horns.png")

juce_add_plugin(${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    COMPANY_NAME "${APP_HUMAN_VENDOR}"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE 
    COPY_PLUGIN_AFTER_BUILD FALSE 
    PLUGIN_MANUFACTURER_CODE HowL
    PLUGIN_CODE SBB1
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "${APP_HUMAN_NAME}"
    ICON_BIG "${app_icon}"  
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
        src/presetmanager.cpp
        src/presetloadui.cpp
        src/presetsaveui.cpp
        src/valuetreetexteditor.cpp
        src/programmanager.cpp
        src/programtablelistboxmodel.cpp
        src/programtablelistbox.cpp
        src/programeditor.cpp
        src/presettablelistbox.cpp
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
    RUNTIME DESTINATION "."
    COMPONENT Standalone
)

set_property(INSTALL "$<TARGET_FILE_NAME:${PROJECT_NAME}_Standalone>"
  PROPERTY CPACK_START_MENU_SHORTCUTS "Sick Beat Betty Standalone"
)

install(TARGETS ${PROJECT_NAME}_VST3
    LIBRARY DESTINATION "."
    COMPONENT VST3
)

include(InstallRequiredSystemLibraries)
install(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} 
    DESTINATION "."
    COMPONENT Runtime
)

set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "Runtime")
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Runtime files needed by both the standalone and plugins")

set(CPACK_COMPONENT_STANDALONE_DISPLAY_NAME "Standalone App")
set(CPACK_COMPONENT_STANDALONE_DESCRIPTION "Version of Sick Beat Betty that runs as a standalone application")
set(CPACK_COMPONENT_STANDALONE_DEPENDS Runtime)

set(CPACK_COMPONENT_VST3_DISPLAY_NAME "VST3 Plugin")
set(CPACK_COMPONENT_VST3_DESCRIPTION "Version of Sick Beat Betty that runs as a VST3 plugin")
set(CPACK_COMPONENT_VST3_DEPENDS Runtime)

set(CPACK_COMPONENTS_ALL VST3 Standalone Runtime)
set(CPACK_COMPONENT_GROUPING ALL_COMPONENTS_IN_ONE)

set(CPACK_PACKAGE_NAME ${APP_HUMAN_NAME})
set(CPACK_PACKAGE_VENDOR ${APP_HUMAN_VENDOR})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${APP_HUMAN_NAME})
#set(CPACK_PACKAGE_DESCRIPTION_FILE "dnescription.txt")
#set(CPACK_RESOURCE_FILE_WELCOME "welcome.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.rtf")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/InstallerConfig.cmake")
set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")

if(WIN32)
    set(WIX_DIR "${CMAKE_SOURCE_DIR}/installer/wix")
    set(CPACK_GENERATOR WIX)
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/icons/sbb.ico")
    # image at top of all pages besides welcome and completion, 493 x 58 px
    set(CPACK_WIX_UI_BANNER    "${WIX_DIR}/sbb_wix_banner.png") 
    # background image on welcome and completion pages, 493 x 312 px
    set(CPACK_WIX_UI_DIALOG    "${WIX_DIR}/sbb_wix_dialog.png") 
    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # Confiuration for 64bit build
        set(CPACK_WIX_UPGRADE_GUID "6541AC2E-DA46-481C-9CF6-FE3CDF3A4C3F")
    else()
        # Configuration for 32bit build
        set(CPACK_WIX_UPGRADE_GUID "6541AC2E-DA46-481C-9CF6-FE3CDF3A4C3E")
    endif()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CPACK_SYSTEM_NAME win64)
    else()
        set(CPACK_SYSTEM_NAME win32)
    endif()
else()    
    set(CPACK_GENERATOR ZIP)    
endif()
set(CPACK_PACKAGE_FILE_NAME "${APP_HUMAN_NAME}-${BUILD_INFO_VERSION}-${CPACK_SYSTEM_NAME}")

include(CodeSign)
code_sign_is_enabled(can_sign)
if(can_sign)
	set(M_SIGN_UAC_URL "https://howardlogic.com")
	configure_file(CMakeModules/CodeSignScript.cmake.in codesign_post.cmake @ONLY)
	list(APPEND CPACK_POST_BUILD_SCRIPTS 
		"${CMAKE_BINARY_DIR}/codesign_post.cmake"
	)
endif()
include(CPack)

# Hack to allow building a installer from within VS Code
ADD_CUSTOM_TARGET(installer
    COMMAND "${CMAKE_CPACK_COMMAND}"
        "-C" "Release"
        "--config" "${CMAKE_BINARY_DIR}/InstallerConfig.cmake"
    COMMENT "Running CPack. Please wait..."
    DEPENDS ${PROJECT_NAME}
)

