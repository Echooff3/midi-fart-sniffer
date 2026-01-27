#include "PluginProcessor.h"
#include "PluginEditor.h"

MidiFartSnifferEditor::MidiFartSnifferEditor (MidiFartSnifferProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // File browser setup
    wildCardFilter = std::make_unique<juce::WildcardFileFilter> ("*.mid;*.midi", "*", "MIDI Files");
    fileBrowser = std::make_unique<juce::FileBrowserComponent> (
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
        wildCardFilter.get(),
        nullptr
    );
    
    fileBrowser->addListener (this);

    addAndMakeVisible (fileBrowser.get());

    // Playback controls
    playButton.onClick = [this] { 
        audioProcessor.startPlayback(); 
        statusLabel.setText ("Playing...", juce::dontSendNotification); 
        updateStatus();
    };
    stopButton.onClick = [this] { 
        audioProcessor.stopPlayback(); 
        statusLabel.setText ("Stopped", juce::dontSendNotification); 
        updateStatus();
    };
    loopButton.onClick = [this] { 
        audioProcessor.setLooping (loopButton.getToggleState()); 
    };
    syncButton.onClick = [this] {
        audioProcessor.setSyncToHost (syncButton.getToggleState());
        updateStatus();
    };
    syncButton.setToggleState (audioProcessor.isSyncedToHost(), juce::dontSendNotification);

    addAndMakeVisible (playButton);
    addAndMakeVisible (stopButton);
    addAndMakeVisible (loopButton);
    addAndMakeVisible (syncButton);

    // Position slider
    positionSlider.setRange (0.0, 1.0, 0.0);
    positionSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    positionSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    positionSlider.setEnabled (false);
    addAndMakeVisible (positionSlider);

    // Status labels
    fileNameLabel.setJustificationType (juce::Justification::centredLeft);
    fileNameLabel.setFont (juce::Font (15.0f));
    statusLabel.setJustificationType (juce::Justification::centredLeft);
    statusLabel.setFont (juce::Font (15.0f));
    tempoLabel.setJustificationType (juce::Justification::centredLeft);
    tempoLabel.setFont (juce::Font (15.0f));

    addAndMakeVisible (fileNameLabel);
    addAndMakeVisible (statusLabel);
    addAndMakeVisible (tempoLabel);

    // Initial status
    updateStatus();

    // Start timer for updating position
    startTimerHz (30);

    setSize (800, 600);
}

MidiFartSnifferEditor::~MidiFartSnifferEditor()
{
    stopTimer();
}

void MidiFartSnifferEditor::timerCallback()
{
    if (audioProcessor.getIsPlaying())
    {
        positionSlider.setValue (audioProcessor.getPlaybackPosition(), juce::dontSendNotification);
        updateStatus();
    }
}

void MidiFartSnifferEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MidiFartSnifferEditor::resized()
{
    auto bounds = getLocalBounds();

    // Left panel: File browser (60% width)
    fileBrowser->setBounds (bounds.removeFromLeft (bounds.proportionOfWidth (0.6f)));

    // Right panel: Controls and status
    auto rightPanel = bounds.removeFromRight (bounds.proportionOfWidth (0.4f));

    // Buttons row
    auto buttonRow = rightPanel.removeFromTop (40);
    playButton.setBounds (buttonRow.removeFromLeft (buttonRow.proportionOfWidth (0.25f)));
    stopButton.setBounds (buttonRow.removeFromLeft (buttonRow.proportionOfWidth (0.25f)));
    loopButton.setBounds (buttonRow.removeFromLeft (buttonRow.proportionOfWidth (0.25f)));
    syncButton.setBounds (buttonRow);

    // Position slider
    positionSlider.setBounds (rightPanel.removeFromTop (30).reduced (5));

    // Status labels
    fileNameLabel.setBounds (rightPanel.removeFromTop (25));
    statusLabel.setBounds (rightPanel.removeFromTop (25));
    tempoLabel.setBounds (rightPanel.removeFromTop (25));
}

void MidiFartSnifferEditor::selectionChanged()
{
    auto selectedFile = fileBrowser->getSelectedFile (0);
    if (selectedFile.existsAsFile())
    {
        loadSelectedFile (selectedFile);
    }
}

void MidiFartSnifferEditor::fileClicked (const juce::File& file, const juce::MouseEvent&)
{
    if (file.existsAsFile())
    {
        loadSelectedFile (file);
    }
}

void MidiFartSnifferEditor::fileDoubleClicked (const juce::File& file)
{
    if (file.existsAsFile())
    {
        loadSelectedFile (file);
    }
}

void MidiFartSnifferEditor::loadSelectedFile (const juce::File& file)
{
    audioProcessor.loadMidiFile (file);
    fileNameLabel.setText (file.getFileName(), juce::dontSendNotification);
    statusLabel.setText ("File loaded. Click Play to start.", juce::dontSendNotification);
    updateStatus();
}

void MidiFartSnifferEditor::updateStatus()
{
    double tempo = audioProcessor.getCurrentTempo();
    bool synced = audioProcessor.isSyncedToHost();
    tempoLabel.setText ("Tempo: " + juce::String (tempo, 1) + " BPM (sync: " + (synced ? "host" : "file") + ")", juce::dontSendNotification);
    statusLabel.setText (audioProcessor.getIsPlaying() ? "Playing" : "Ready", juce::dontSendNotification);
}
