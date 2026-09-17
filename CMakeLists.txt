cmake_minimum_required(VERSION 3.22)

project(Helm
    VERSION 1.0.0
    DESCRIPTION "A free polyphonic synthesizer with lots of modulation"
    HOMEPAGE_URL "https://github.com/keithadler/helm"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(MSVC)
    # Link the C++ runtime statically so the zip needs no Visual C++ redistributable.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

option(HELM_BUILD_PLUGINS    "Build the VST3, AU, LV2 and CLAP plugins" ON)
option(HELM_BUILD_STANDALONE "Build the standalone application" ON)
option(HELM_BUILD_TESTS      "Build the tests" ON)

# ---- JUCE and the CLAP extensions, fetched at configure time ----
# JUCE is AGPLv3 for open-source use; Helm is GPLv3, which the AGPL accepts (GPLv3 section 13).
include(FetchContent)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.15
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(juce)
if(HELM_BUILD_PLUGINS)
    FetchContent_Declare(clap-juce-extensions
        GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
        GIT_TAG        0.26.0
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(clap-juce-extensions)
endif()

# ---- resources: fonts and images become BinaryData, named as the jucer named them ----
file(GLOB HELM_FONTS  CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fonts/*.ttf)
file(GLOB HELM_IMAGES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/images/*.png ${CMAKE_CURRENT_SOURCE_DIR}/images/*.xpm)
juce_add_binary_data(HelmData SOURCES ${HELM_FONTS} ${HELM_IMAGES})

# ---- the synth: engine (mopo) plus the shared editor and synthesis code ----
file(GLOB HELM_MOPO_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mopo/src/*.cpp)
file(GLOB HELM_SHARED_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/synthesis/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/editor_components/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/editor_sections/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/look_and_feel/*.cpp)
set(HELM_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/mopo/src
    ${CMAKE_CURRENT_SOURCE_DIR}/concurrentqueue
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/synthesis
    ${CMAKE_CURRENT_SOURCE_DIR}/src/editor_components
    ${CMAKE_CURRENT_SOURCE_DIR}/src/editor_sections
    ${CMAKE_CURRENT_SOURCE_DIR}/src/look_and_feel)
set(HELM_JUCE_MODULES
    juce::juce_audio_basics juce::juce_audio_devices juce::juce_audio_formats juce::juce_audio_processors
    juce::juce_audio_utils juce::juce_core juce::juce_data_structures juce::juce_events juce::juce_graphics
    juce::juce_gui_basics juce::juce_gui_extra juce::juce_opengl)
set(HELM_JUCE_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_XRANDR=0
    # Helm's file dialogs and alerts are synchronous, as they were in JUCE 5; JUCE 7 made that opt-in.
    JUCE_MODAL_LOOPS_PERMITTED=1
    HELM_VERSION="${PROJECT_VERSION}")

function(helm_configure_target target)
    target_sources(${target} PRIVATE ${HELM_MOPO_SOURCES} ${HELM_SHARED_SOURCES} ${ARGN})
    target_include_directories(${target} PRIVATE ${HELM_INCLUDE_DIRS})
    target_compile_definitions(${target} PRIVATE ${HELM_JUCE_DEFINES})
    target_link_libraries(${target} PRIVATE HelmData ${HELM_JUCE_MODULES}
        PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags)
    # The sources include the jucer-era umbrella header; JUCE writes one per target.
    juce_generate_juce_header(${target})
    if(NOT MSVC)
        # ScopedPointer is deprecated in JUCE 8 but still shipped; Helm uses it in some 230 places, and
        # replacing it with std::unique_ptr is its own change. Until then the warning is noise.
        target_compile_options(${target} PRIVATE -Wno-unused-parameter -Wno-sign-compare -Wno-deprecated-declarations)
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${target} PRIVATE -ferror-limit=0)
    endif()
endfunction()

# ---- plugins: VST3, AU, LV2 through JUCE; CLAP through clap-juce-extensions ----
if(HELM_BUILD_PLUGINS)
    set(HELM_FORMATS VST3 LV2)
    if(APPLE)
        list(APPEND HELM_FORMATS AU)
    endif()
    juce_add_plugin(Helm
        PRODUCT_NAME "Helm"
        COMPANY_NAME "Matt Tytel"
        COMPANY_WEBSITE "https://github.com/keithadler/helm"
        BUNDLE_ID "org.tytel.helmplugin"
        PLUGIN_MANUFACTURER_CODE Tyte
        PLUGIN_CODE Helm
        FORMATS ${HELM_FORMATS}
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        VST3_CATEGORIES Instrument Synth
        AU_MAIN_TYPE kAudioUnitType_MusicDevice
        AU_EXPORT_PREFIX helm
        LV2URI "http://tytel.org/helm"
        COPY_PLUGIN_AFTER_BUILD FALSE)
    helm_configure_target(Helm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin/helm_plugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin/helm_editor.cpp)
    target_include_directories(Helm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin)
    clap_juce_extensions_plugin(TARGET Helm
        CLAP_ID "org.tytel.helm"
        CLAP_FEATURES instrument synthesizer "virtual analog")
endif()

# ---- the standalone application, Helm's own (menus, computer keyboard), not JUCE's generic shell ----
if(HELM_BUILD_STANDALONE)
    juce_add_gui_app(HelmStandalone
        PRODUCT_NAME "Helm"
        COMPANY_NAME "Matt Tytel"
        BUNDLE_ID "org.tytel.helm"
        ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/images/helm_icon_512_1x.png"
        ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/images/helm_icon_128_1x.png")
    helm_configure_target(HelmStandalone
        ${CMAKE_CURRENT_SOURCE_DIR}/src/standalone/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/standalone/helm_editor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/standalone/helm_computer_keyboard.cpp)
    target_include_directories(HelmStandalone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/standalone)
    target_compile_definitions(HelmStandalone PRIVATE JUCE_APPLICATION_NAME_STRING="Helm" JUCE_APPLICATION_VERSION_STRING="${PROJECT_VERSION}")
endif()
