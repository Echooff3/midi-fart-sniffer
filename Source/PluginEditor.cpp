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
    
    autoPlayCheckbox.onClick = [this] {
        audioProcessor.setAutoPlay (autoPlayCheckbox.getToggleState());
    };
    autoPlayCheckbox.setToggleState (audioProcessor.isAutoPlayEnabled(), juce::dontSendNotification);
    
    favoriteButton.onClick = [this] {
        toggleFavorite();
    };

    addAndMakeVisible (playButton);
    addAndMakeVisible (stopButton);
    addAndMakeVisible (loopButton);
    addAndMakeVisible (syncButton);
    addAndMakeVisible (autoPlayCheckbox);
    addAndMakeVisible (favoriteButton);

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
    
    // Favorites list setup
    favoritesLabel.setJustificationType (juce::Justification::centredLeft);
    favoritesLabel.setFont (juce::Font (15.0f, juce::Font::bold));
    addAndMakeVisible (favoritesLabel);
    
    favoritesList.setModel (this);
    favoritesList.setRowHeight (20);
    addAndMakeVisible (favoritesList);
    
    updateFavoritesList();

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
    auto rightPanel = bounds.reduced (5);

    // Buttons row 1
    auto buttonRow1 = rightPanel.removeFromTop (30);
    playButton.setBounds (buttonRow1.removeFromLeft (buttonRow1.proportionOfWidth (0.5f)).reduced (2));
    stopButton.setBounds (buttonRow1.reduced (2));
    
    // Buttons row 2
    auto buttonRow2 = rightPanel.removeFromTop (30);
    loopButton.setBounds (buttonRow2.removeFromLeft (buttonRow2.proportionOfWidth (0.5f)).reduced (2));
    syncButton.setBounds (buttonRow2.reduced (2));
    
    // Auto-play checkbox
    autoPlayCheckbox.setBounds (rightPanel.removeFromTop (30).reduced (2));
    
    // Favorite button
    favoriteButton.setBounds (rightPanel.removeFromTop (30).reduced (2));

    // Position slider
    positionSlider.setBounds (rightPanel.removeFromTop (30).reduced (5));

    // Status labels
    fileNameLabel.setBounds (rightPanel.removeFromTop (25).reduced (2));
    statusLabel.setBounds (rightPanel.removeFromTop (25).reduced (2));
    tempoLabel.setBounds (rightPanel.removeFromTop (25).reduced (2));
    
    // Favorites section
    rightPanel.removeFromTop (10); // spacing
    favoritesLabel.setBounds (rightPanel.removeFromTop (25).reduced (2));
    favoritesList.setBounds (rightPanel.reduced (2));
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
        // Check if clicking the same file that is currently playing
        if (audioProcessor.getIsPlaying() && file == lastClickedFile)
        {
            // Stop playback if clicking the same file again
            audioProcessor.stopPlayback();
            statusLabel.setText ("Stopped", juce::dontSendNotification);
            updateStatus();
        }
        else
        {
            // Load the file
            loadSelectedFile (file);
            lastClickedFile = file;
            
            // Auto-play if enabled
            if (audioProcessor.isAutoPlayEnabled())
            {
                audioProcessor.startPlayback();
                statusLabel.setText ("Playing...", juce::dontSendNotification);
                updateStatus();
            }
        }
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
    
    // Update favorite button text based on current file
    auto currentFile = audioProcessor.getCurrentFile();
    if (currentFile.existsAsFile() && audioProcessor.isFavorite (currentFile))
    {
        favoriteButton.setButtonText ("★ Unfavorite");
    }
    else
    {
        favoriteButton.setButtonText ("☆ Favorite");
    }
}

void MidiFartSnifferEditor::updateFavoritesList()
{
    favoritesArray = audioProcessor.getFavorites();
    favoritesList.updateContent();
    favoritesList.repaint();
}

void MidiFartSnifferEditor::toggleFavorite()
{
    auto currentFile = audioProcessor.getCurrentFile();
    if (currentFile.existsAsFile())
    {
        if (audioProcessor.isFavorite (currentFile))
        {
            audioProcessor.removeFromFavorites (currentFile);
        }
        else
        {
            audioProcessor.addToFavorites (currentFile);
        }
        updateFavoritesList();
        updateStatus();
    }
}

// ListBoxModel methods
int MidiFartSnifferEditor::getNumRows()
{
    return favoritesArray.size();
}

void MidiFartSnifferEditor::paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll (juce::Colours::lightblue);
    else
        g.fillAll (juce::Colours::white);
    
    g.setColour (juce::Colours::black);
    g.setFont (12.0f);
    
    if (rowNumber < favoritesArray.size())
    {
        juce::File file (favoritesArray[rowNumber]);
        g.drawText (file.getFileName(), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
}

void MidiFartSnifferEditor::listBoxItemClicked (int row, const juce::MouseEvent& e)
{
    if (row < favoritesArray.size())
    {
        juce::File file (favoritesArray[row]);
        if (file.existsAsFile())
        {
            // Check if clicking the same file that is currently playing
            if (audioProcessor.getIsPlaying() && file == lastClickedFile)
            {
                // Stop playback if clicking the same file again
                audioProcessor.stopPlayback();
                statusLabel.setText ("Stopped", juce::dontSendNotification);
                updateStatus();
            }
            else
            {
                // Load and play the favorite file
                loadSelectedFile (file);
                lastClickedFile = file;
                
                // Always start playback when clicking a favorite
                audioProcessor.startPlayback();
                statusLabel.setText ("Playing...", juce::dontSendNotification);
                updateStatus();
            }
        }
    }
}
