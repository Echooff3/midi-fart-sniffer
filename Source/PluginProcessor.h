#pragma once

#include <JuceHeader.h>

class MidiFartSnifferEditor;

class MidiFartSnifferProcessor final : public juce::AudioProcessor
{
public:
    MidiFartSnifferProcessor();
    ~MidiFartSnifferProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#if ! JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override {}
    const juce::String getProgramName (int index) override { return {}; }
    void changeProgramName (int index, const juce::String& newName) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Custom methods
    void setSyncToHost (bool shouldSync);
    bool isSyncedToHost() const { return syncToHost; }
    double getCurrentTempo() const;
    void loadMidiFile (const juce::File& file);
    void startPlayback();
    void stopPlayback();
    void setLooping (bool loop);
    bool getIsPlaying() const;
    double getFileTempo() const;
    int64_t getCurrentTick() const { return currentTick; }
    int64_t getMaxTick() const;
    double getPlaybackPosition() const;

private:
    // MIDI file playback state
    std::unique_ptr<juce::MidiFile> currentMidiFile;
    std::vector<juce::MidiMessageSequence> midiTracks;
    int64_t currentTick = 0;
    bool isPlaying = false;
    bool shouldLoop = false;
    double fileTempo = 120.0;
    double hostTempo = 120.0;
    bool syncToHost = true;

    double ticksPerQuarterNote = 480.0;
    double samplesPerTick = 0.0;

    std::unique_ptr<juce::MidiOutput> midiOutput;

    void updateHostTempo();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiFartSnifferProcessor)
};
