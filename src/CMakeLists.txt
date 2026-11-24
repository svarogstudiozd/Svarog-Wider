juce_add_plugin(SvarogWider
    COMPANY_NAME "Svarog Studio"
    COMPANY_COPYRIGHT "Svarog Studio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE

    PLUGIN_MANUFACTURER_CODE Svrg
    PLUGIN_CODE Swdr

    FORMATS
        Standalone
        AU
        VST3

    VST3_CATEGORIES "Fx"
    AU_MAIN_TYPE "kAudioUnitType_Effect"

    PRODUCT_NAME "Svarog Wider"
    BUNDLE_ID "com.svarogstudio.svarogwider"
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_compile_features(SvarogWider PUBLIC cxx_std_17)

target_compile_definitions(SvarogWider
    PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_sources(SvarogWider PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp
    SvarogLookAndFeel.cpp
    LevelMeterComponent.cpp
    DraggableValueBox.cpp
)

target_link_libraries(SvarogWider
    PUBLIC
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    PRIVATE
    SvarogWider_Assets
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

juce_generate_juce_header(SvarogWider)