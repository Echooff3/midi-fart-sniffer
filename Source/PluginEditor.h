#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

class MidiFartSnifferEditor final : public juce::AudioProcessorEditor,
                                   private juce::FileBrowserListener,
                                   private juce::Timer
{
public:
    explicit MidiFartSnifferEditor (MidiFartSnifferProcessor&);
    ~MidiFartSnifferEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Timer callback for updating playback position
    void timerCallback() override;

private:
    //==============================================================================
    // FileBrowserListener callbacks
    void selectionChanged() override;
    void fileClicked (const juce::File& file, const juce::MouseEvent&) override;
    void fileDoubleClicked (const juce::File&) override;

    // Custom methods
    void loadSelectedFile (const juce::File& file);
    void updateStatus();

    //==============================================================================
    MidiFartSnifferProcessor& audioProcessor;

    std::unique_ptr<juce::FileBrowserComponent> fileBrowser;

    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::ToggleButton loopButton { "Loop" };
    juce::ToggleButton syncButton { "Sync to Host" };

    juce::Slider positionSlider { juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };

    juce::Label fileNameLabel { {}, "No file selected" };
    juce::Label statusLabel { {}, "Ready" };
    juce::Label tempoLabel { {}, "Tempo: -- BPM" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiFartSnifferEditor)
};
