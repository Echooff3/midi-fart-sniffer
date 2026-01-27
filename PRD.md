# Product Requirements Document (PRD)
## MIDI File Player VST Plugin

**Project Name:** MidiFartSniffer  
**Version:** 1.0.0  
**Framework:** JUCE  
**Plugin Type:** VST3 (with potential for AU and Standalone formats)  
**Date:** 2026-01-27

---

## 1. Executive Summary

MidiFartSniffer is a VST plugin designed for browsing and playing MIDI files, specifically optimized for managing large collections of drum MIDI patterns. The plugin provides an intuitive file browser interface that allows users to navigate folders containing MIDI files, select files, and stream the MIDI data out to the host application or external MIDI devices. This tool is ideal for producers and musicians who need to quickly audition and trigger drum patterns from their MIDI library.

---

## 2. Product Goals

### Primary Goals
- Provide a streamlined interface for browsing MIDI files from local directories
- Enable real-time playback of selected MIDI files with accurate timing
- Output MIDI messages to the host DAW and/or external MIDI devices
- Support efficient navigation of large MIDI file collections (hundreds of files)

### Secondary Goals
- Maintain low CPU overhead during playback
- Provide visual feedback for currently playing MIDI events
- Support common MIDI file formats (Format 0, Format 1)
- Offer playback controls (play, stop, loop)

---

## 3. Target Audience

- **Primary:** Music producers and composers working with drum MIDI libraries
- **Secondary:** Sound designers, beatmakers, and electronic musicians
- **Use Case:** Quickly auditioning and triggering drum patterns during production workflow

---

## 4. Functional Requirements

### 4.1 File Browser Component

| ID | Requirement | Description | Priority |
|----|-------------|-------------|----------|
| FR-1 | Directory Navigation | Users can navigate through the file system to select a folder containing MIDI files | P0 |
| FR-2 | MIDI File Display | The file browser displays only `.mid` and `.midi` files in the selected directory | P0 |
| FR-3 | File Selection | Users can click on a MIDI file in the list to select it for playback | P0 |
| FR-4 | Folder Persistence | The plugin remembers the last accessed folder between sessions | P1 |
| FR-5 | File Preview | Hovering over a file shows basic metadata (file name, size) | P2 |

**Technical Implementation:**
- Use JUCE's [`FileBrowserComponent`](https://docs.juce.com/master/classFileBrowserComponent.html) or [`FileTreeComponent`](https://docs.juce.com/master/classFileTreeComponent.html)
- Configure with `FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles`
- Filter files using wildcard pattern `"*.mid;*.midi"`
- Store last directory path using JUCE's [`PropertiesFile`](https://docs.juce.com/master/classPropertiesFile.html) or [`ApplicationProperties`](https://docs.juce.com/master/classApplicationProperties.html)

### 4.2 MIDI File Loading

| ID | Requirement | Description | Priority |
|----|-------------|-------------|----------|
| FR-6 | File Parsing | The plugin can parse standard MIDI file formats (Format 0 and Format 1) | P0 |
| FR-7 | Error Handling | Invalid or corrupted MIDI files display an error message without crashing | P0 |
| FR-8 | Load Time | MIDI files load within 100ms for typical drum patterns (< 50KB) | P1 |
| FR-9 | Tempo Detection | The plugin detects and displays the tempo from the MIDI file if available | P2 |

**Technical Implementation:**
- Use JUCE's [`MidiFile`](https://docs.juce.com/master/classMidiFile.html) class for parsing
- Call [`MidiFile::readFrom()`](https://docs.juce.com/master/classMidiFile.html#a8e8c5e8e8e8e8e8e8e8e8e8e8e8e8e8) to load files
- Validate file format using [`MidiFile::getNumTracks()`](https://docs.juce.com/master/classMidiFile.html#a8e8c5e8e8e8e8e8e8e8e8e8e8e8e8e8)
- Extract tempo from meta events (FF 51 03)

### 4.3 MIDI Playback

| ID | Requirement | Description | Priority |
|----|-------------|-------------|----------|
| FR-10 | Playback Trigger | Clicking a MIDI file immediately starts playback | P0 |
| FR-11 | Stop Control | A stop button halts current playback | P0 |
| FR-12 | Loop Toggle | Users can enable/disable looping of the current MIDI file | P1 |
| FR-13 | Playback Position | Visual indicator shows current playback position within the file | P1 |
| FR-14 | Multiple Tracks | All tracks in Format 1 files are played simultaneously | P0 |
| FR-15 | Host BPM Sync | Playback tempo automatically syncs to host DAW BPM | P0 |
| FR-16 | Tempo Mode Toggle | Users can switch between host sync and file tempo mode | P1 |

**Technical Implementation:**
- Implement playback timer using JUCE's [`HighResolutionTimer`](https://docs.juce.com/master/classHighResolutionTimer.html) or audio thread timing
- Convert MIDI time to sample positions based on host sample rate
- Use [`MidiMessage::getTimeStamp()`](https://docs.juce.com/master/classMidiMessage.html#a8e8c5e8e8e8e8e8e8e8e8e8e8e8e8e8) for event timing
- Track playback position with a sample counter
- Retrieve host tempo using [`AudioPlayHead::getPosition()`](https://docs.juce.com/master/classAudioPlayHead.html) which returns [`PositionInfo`](https://docs.juce.com/master/structAudioPlayHead_1_1PositionInfo.html)
- Access tempo via [`PositionInfo::getBpm()`](https://docs.juce.com/master/structAudioPlayHead_1_1PositionInfo.html) method
- Convert MIDI file ticks to time based on current tempo (host or file)
- Handle tempo changes in real-time during playback

### 4.4 MIDI Output

| ID | Requirement | Description | Priority |
|----|-------------|-------------|----------|
| FR-17 | Host Output | MIDI messages are sent to the host DAW via the plugin's MIDI output | P0 |
| FR-18 | External Device Output | Users can select external MIDI output devices for direct routing | P1 |
| FR-19 | Channel Selection | Users can specify which MIDI channel(s) to output on | P2 |
| FR-20 | Velocity Scaling | Optional velocity scaling control (0-200%) | P2 |

**Technical Implementation:**
- Send MIDI to host via [`AudioProcessor::processBlock()`](https://docs.juce.com/master/classAudioProcessor.html#a8e8c5e8e8e8e8e8e8e8e8e8e8e8e8e8) using the [`MidiBuffer`](https://docs.juce.com/master/classMidiBuffer.html)
- Use [`MidiOutput::openDevice()`](https://docs.juce.com/master/classMidiOutput.html#a8e8c5e8e8e8e8e8e8e8e8e8e8e8e8e8) and [`MidiOutput::sendMessageNow()`](https://docs.juce.com/master/classMidiOutput.html#a8e8c5e8e8e8e8e8e8e8e8e8e8e8e8e8) for external devices
- Implement channel mapping in the message processing loop

### 4.5 User Interface

| ID | Requirement | Description | Priority |
|----|-------------|-------------|----------|
| FR-21 | File Browser Panel | Left panel displays the file browser with folder navigation | P0 |
| FR-22 | Control Panel | Right panel contains playback controls (Play, Stop, Loop) | P0 |
| FR-23 | Status Display | Shows currently loaded file name and playback status | P0 |
| FR-24 | Tempo Display | Shows current tempo (host BPM or file BPM) with sync indicator | P0 |
| FR-25 | Sync Toggle | Button to switch between host sync and file tempo mode | P1 |
| FR-26 | MIDI Monitor | Optional display of outgoing MIDI messages | P2 |
| FR-27 | Resizable Window | Plugin window is resizable with minimum dimensions | P1 |

**Technical Implementation:**
- Use JUCE's [`Component`](https://docs.juce.com/master/classComponent.html) hierarchy for layout
- Implement [`resized()`](https://docs.juce.com/master/classComponent.html#a8e8c5e8e8e8e8e8e8e8e8e8e8e8e8e8) for responsive layout
- Use [`FlexBox`](https://docs.juce.com/master/classFlexBox.html) or [`Grid`](https://docs.juce.com/master/classGrid.html) for modern layout
- Style with [`LookAndFeel`](https://docs.juce.com/master/classLookAndFeel.html) for consistent appearance

---

## 5. Non-Functional Requirements

### 5.1 Performance
| ID | Requirement | Metric |
|----|-------------|--------|
| NFR-1 | CPU Usage | < 1% during idle, < 5% during playback |
| NFR-2 | Memory Usage | < 50MB baseline, < 100MB with large MIDI files loaded |
| NFR-3 | Latency | < 5ms from file selection to first MIDI output |
| NFR-4 | File Loading | < 100ms for typical drum MIDI files |

### 5.2 Compatibility
| ID | Requirement | Details |
|----|-------------|---------|
| NFR-5 | Plugin Formats | VST3 (primary), AU (secondary), Standalone (optional) |
| NFR-6 | Operating Systems | Windows 10+, macOS 10.15+, Linux (Ubuntu 20.04+) |
| NFR-7 | DAW Compatibility | Tested with Ableton Live, FL Studio, Logic Pro, Reaper, Cubase |
| NFR-8 | MIDI Standards | MIDI 1.0 bytestream format (Format 0 and Format 1 files) |

### 5.3 Reliability
| ID | Requirement | Details |
|----|-------------|---------|
| NFR-9 | Error Recovery | Graceful handling of file access errors, invalid MIDI data |
| NFR-10 | Thread Safety | All GUI operations on message thread, audio operations on audio thread |
| NFR-11 | Memory Safety | No memory leaks, proper RAII usage throughout |

---

## 6. Technical Architecture

### 6.1 Component Structure

```
MidiFartSnifferPlugin (AudioProcessor)
├── MidiFartSnifferEditor (AudioProcessorEditor)
│   ├── FileBrowserPanel (Component)
│   │   ├── FileTreeComponent (FileBrowserComponent)
│   │   └── DirectoryLabel (Label)
│   ├── ControlPanel (Component)
│   │   ├── PlayButton (TextButton)
│   │   ├── StopButton (TextButton)
│   │   ├── LoopToggle (ToggleButton)
│   │   └── SyncToggle (ToggleButton)
│   ├── StatusPanel (Component)
│   │   ├── FileNameLabel (Label)
│   │   ├── PlaybackStatusLabel (Label)
│   │   └── TempoLabel (Label)
│   └── MidiMonitor (TextEditor) [Optional]
└── MidiFilePlayer (Internal Class)
    ├── MidiFile (JUCE class)
    ├── PlaybackState (enum)
    ├── CurrentPosition (int64 samples)
    └── MidiOutputDevice (MidiOutput*)
```

### 6.2 Key Classes

#### AudioProcessor Subclass
```cpp
class MidiFartSnifferProcessor : public juce::AudioProcessor
{
public:
    MidiFartSnifferProcessor();
    ~MidiFartSnifferProcessor() override;
    
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    
    // State management
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
    // Tempo control
    void setSyncToHost(bool shouldSync);
    bool isSyncedToHost() const { return syncToHost; }
    double getCurrentTempo() const;
    
private:
    // MIDI file playback state
    std::unique_ptr<juce::MidiFile> currentMidiFile;
    int64 playbackPosition = 0;
    bool isPlaying = false;
    bool shouldLoop = false;
    double fileTempo = 120.0;  // Tempo from MIDI file
    double hostTempo = 120.0;    // Tempo from host DAW
    bool syncToHost = true;     // Default to host sync
    
    // Timing conversion
    double ticksPerQuarterNote = 480.0;  // Standard MIDI resolution
    double samplesPerTick = 0.0;
    
    // External MIDI output
    std::unique_ptr<juce::MidiOutput> midiOutput;
    
    // Update host tempo from AudioPlayHead
    void updateHostTempo();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiFartSnifferProcessor)
};
```

#### AudioProcessorEditor Subclass
```cpp
class MidiFartSnifferEditor : public juce::AudioProcessorEditor,
                              private juce::FileBrowserListener
{
public:
    explicit MidiFartSnifferEditor(MidiFartSnifferProcessor&);
    ~MidiFartSnifferEditor() override;
    
    void resized() override;
    void paint(juce::Graphics&) override;
    
private:
    // FileBrowserListener callbacks
    void selectionChanged() override;
    void fileClicked(const juce::File&, const juce::MouseEvent&) override;
    void fileDoubleClicked(const juce::File&) override;
    
    MidiFartSnifferProcessor& processorRef;
    
    // GUI Components
    std::unique_ptr<juce::FileBrowserComponent> fileBrowser;
    juce::TextButton playButton, stopButton;
    juce::ToggleButton loopButton, syncButton;
    juce::Label fileNameLabel, statusLabel, tempoLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiFartSnifferEditor)
};
```

### 6.3 Data Flow

```
User Action (File Click)
    ↓
FileBrowserListener::fileClicked()
    ↓
Editor loads MIDI file via MidiFile::readFrom()
    ↓
Editor extracts file tempo from MIDI meta events
    ↓
Editor notifies Processor of new file
    ↓
Processor prepares playback state
    ↓
Audio Thread: processBlock() called
    ↓
Processor calls updateHostTempo() via AudioPlayHead::getPosition()
    ↓
Processor determines effective tempo (host or file based on sync mode)
    ↓
Processor calculates samplesPerTick based on current tempo and sample rate
    ↓
Processor reads MIDI events at current position
    ↓
Events added to MidiBuffer for host output
    ↓
Events sent to external MidiOutput (if configured)
    ↓
Playback position advances by samplesPerTick
    ↓
Loop check: reset to start if enabled and end reached
```

### 6.4 Host BPM Synchronization

The plugin synchronizes playback tempo with the host DAW using JUCE's [`AudioPlayHead`](https://docs.juce.com/master/classAudioPlayHead.html) interface:

```cpp
void MidiFartSnifferProcessor::updateHostTempo()
{
    if (auto* playHead = getPlayHead())
    {
        auto positionInfo = playHead->getPosition();
        
        if (positionInfo.hasValue())
        {
            auto bpm = positionInfo->getBpm();
            
            if (bpm.hasValue())
            {
                hostTempo = *bpm;
                
                // Recalculate timing when tempo changes
                if (syncToHost && getSampleRate() > 0)
                {
                    samplesPerTick = (60.0 / hostTempo) * getSampleRate() / ticksPerQuarterNote;
                }
            }
        }
    }
}

double MidiFartSnifferProcessor::getCurrentTempo() const
{
    return syncToHost ? hostTempo : fileTempo;
}
```

**Tempo Calculation:**
- Host BPM retrieved via [`AudioPlayHead::getPosition()->getBpm()`](https://docs.juce.com/master/structAudioPlayHead_1_1PositionInfo.html)
- File BPM extracted from MIDI Set Tempo meta event (0xFF 0x51 0x03)
- Samples per tick calculated as: `(60 / tempo) × sampleRate / ticksPerQuarterNote`
- Real-time tempo updates handled in each [`processBlock()`](https://docs.juce.com/master/classAudioProcessor.html#a8e8c5e8e8e8e8e8e8e8e8e8e8e8e8e8) call

---

## 7. Implementation Phases

### Phase 1: Core Functionality (MVP)
- [ ] Set up JUCE plugin project structure
- [ ] Implement basic AudioProcessor with MIDI output
- [ ] Create FileBrowserComponent for MIDI file selection
- [ ] Implement MIDI file loading using MidiFile class
- [ ] Basic playback: click file → play through once
- [ ] MIDI output to host via processBlock()

### Phase 2: Enhanced Playback
- [ ] Add stop button functionality
- [ ] Implement loop toggle
- [ ] Add playback position tracking
- [ ] Visual feedback for playing state
- [ ] Handle multiple tracks in Format 1 files
- [ ] Implement host BPM synchronization via AudioPlayHead
- [ ] Add tempo sync toggle (host vs file tempo)
- [ ] Display current tempo with sync indicator

### Phase 3: External MIDI Output
- [ ] Add MIDI output device selector
- [ ] Implement external device connection
- [ ] Route MIDI to both host and external device
- [ ] Add channel selection option

### Phase 4: Polish & UX
- [ ] Persist last directory between sessions
- [ ] Add file metadata display (tempo, track count)
- [ ] Implement optional MIDI monitor
- [ ] Add keyboard shortcuts
- [ ] Error handling for invalid files
- [ ] Performance optimization

### Phase 5: Testing & Release
- [ ] Unit tests for MIDI file parsing
- [ ] Integration testing with major DAWs
- [ ] Performance profiling
- [ ] Documentation
- [ ] Build for all target platforms

---

## 8. Dependencies

### JUCE Modules Required
- `juce_audio_basics` - Core audio and MIDI classes
- `juce_audio_devices` - MIDI input/output device management
- `juce_audio_formats` - MIDI file format support
- `juce_audio_processors` - Plugin framework
- `juce_core` - Core utilities
- `juce_data_structures` - Data structures
- `juce_events` - Timers and messaging
- `juce_graphics` - Graphics and rendering
- `juce_gui_basics` - GUI components

### External Dependencies
- None (pure JUCE implementation)

---

## 9. Success Criteria

The project will be considered successful when:

1. **Functional Completeness**
   - Users can browse folders and select MIDI files
   - Selected files play back with accurate timing
   - MIDI output reaches the host DAW correctly
   - Basic controls (play, stop, loop) work as expected

2. **Performance**
   - CPU usage remains under 5% during playback
   - File loading completes within 100ms for typical files
   - No audio dropouts or glitches during playback

3. **User Experience**
   - Intuitive interface requiring minimal learning curve
   - Responsive UI with no lag during file navigation
   - Clear visual feedback for all user actions

4. **Stability**
   - No crashes when loading invalid MIDI files
   - Graceful handling of file system errors
   - Stable operation across multiple DAW platforms

---

## 10. Future Enhancements (Out of Scope for v1.0)

- MIDI file editing capabilities (note editing, quantization)
- Playlist/queue functionality for multiple files
- MIDI file search and filtering
- Custom file tagging and categorization
- Drag and drop from DAW
- MIDI learn for external controllers
- Preset system for common configurations
- MIDI 2.0 / UMP support
- Network MIDI output
- Visualization of MIDI events (piano roll display)

---

## 11. Appendix

### 11.1 Glossary

| Term | Definition |
|------|------------|
| VST | Virtual Studio Technology - Steinberg's plugin interface standard |
| DAW | Digital Audio Workstation - Software for music production |
| MIDI | Musical Instrument Digital Interface - Protocol for electronic instruments |
| Format 0 | Single-track MIDI file format |
| Format 1 | Multi-track MIDI file format |
| JUCE | C++ framework for cross-platform audio applications |

### 11.2 References

- [JUCE Documentation](https://docs.juce.com/)
- [JUCE MIDI Tutorial](https://docs.juce.com/master/tutorial_midi_message.html)
- [JUCE Audio Processor Tutorial](https://docs.juce.com/master/tutorial_audio_processor.html)
- [MIDI File Format Specification](https://www.midi.org/specifications-old/item/the-midi-1-0-specification)

---

**Document Version:** 1.0  
**Last Updated:** 2026-01-27  
**Status:** Draft - Ready for Review
