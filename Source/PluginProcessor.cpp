#include "PluginProcessor.h"
#include "PluginEditor.h"

MidiFartSnifferProcessor::MidiFartSnifferProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
}

MidiFartSnifferProcessor::~MidiFartSnifferProcessor()
{
}

void MidiFartSnifferProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentTick = 0;
}

void MidiFartSnifferProcessor::releaseResources()
{
    // When playback stops, you can use this opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MidiFartSnifferProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo
    // channels, regardless of the number of buses.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MidiFartSnifferProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be handled by the DAW)
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Playback logic
    if (isPlaying && ! midiTracks.empty())
    {
        updateHostTempo();

        double tempo = getCurrentTempo();
        double sampleRate = getSampleRate();
        if (sampleRate > 0.0)
        {
            double secondsPerBeat = 60.0 / tempo;
            double samplesPerBeat = secondsPerBeat * sampleRate;
            double ticksPerBeat = ticksPerQuarterNote;
            samplesPerTick = samplesPerBeat / ticksPerBeat;
        }

        int numSamples = buffer.getNumSamples();
        int64_t ticksToAdvance = static_cast<int64_t> (numSamples / samplesPerTick + 0.5); // round to nearest

        int64_t startTick = currentTick;
        int64_t endTick = currentTick + ticksToAdvance;

        // Add events from all tracks
        for (const auto& track : midiTracks)
        {
            for (int i = 0; i < track.getNumEvents(); ++i)
            {
                auto* event = track.getEventPointer (i);
                if (event == nullptr)
                    continue;

                int64_t eventTick = static_cast<int64_t> (event->message.getTimeStamp());
                if (eventTick >= startTick && eventTick < endTick)
                {
                    int sampleOffset = static_cast<int> ((eventTick - startTick) * samplesPerTick + 0.5);
                    if (sampleOffset >= 0 && sampleOffset < numSamples)
                    {
                        midiMessages.addEvent (event->message, sampleOffset);
                    }
                }
                else if (eventTick >= endTick)
                {
                    break;
                }
            }
        }

        currentTick += ticksToAdvance;

        // Check if end reached
        int64_t maxTick = getMaxTick();
        if (currentTick >= maxTick)
        {
            if (shouldLoop)
                currentTick = 0;
            else
                isPlaying = false;
        }
    }
}

juce::AudioProcessorEditor* MidiFartSnifferProcessor::createEditor()
{
    return new MidiFartSnifferEditor (*this);
}

void MidiFartSnifferProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Create XML to store state
    std::unique_ptr<juce::XmlElement> xml (new juce::XmlElement ("MidiFartSnifferState"));
    
    xml->setAttribute ("autoPlay", autoPlayEnabled);
    
    // Save favorites
    auto* favoritesElement = xml->createNewChildElement ("Favorites");
    for (const auto& fav : favoriteFiles)
    {
        favoritesElement->createNewChildElement ("File")->setAttribute ("path", fav);
    }
    
    copyXmlToBinary (*xml, destData);
}

void MidiFartSnifferProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore state from XML
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName ("MidiFartSnifferState"))
        {
            autoPlayEnabled = xmlState->getBoolAttribute ("autoPlay", false);
            
            // Restore favorites
            favoriteFiles.clear();
            auto* favoritesElement = xmlState->getChildByName ("Favorites");
            if (favoritesElement != nullptr)
            {
                for (auto* fileElement : favoritesElement->getChildIterator())
                {
                    if (fileElement->hasTagName ("File"))
                    {
                        favoriteFiles.add (fileElement->getStringAttribute ("path"));
                    }
                }
            }
        }
    }
}

void MidiFartSnifferProcessor::updateHostTempo()
{
    if (auto* playHead = getPlayHead())
    {
        auto pos = playHead->getPosition();

        if (pos.hasValue())
        {
            auto bpm = pos->getBpm();
            if (bpm.hasValue())
            {
                hostTempo = *bpm;
            }
        }
    }
}

double MidiFartSnifferProcessor::getCurrentTempo() const
{
    return syncToHost ? hostTempo : fileTempo;
}

void MidiFartSnifferProcessor::setSyncToHost (bool shouldSync)
{
    syncToHost = shouldSync;
}

void MidiFartSnifferProcessor::loadMidiFile (const juce::File& file)
{
    currentFile = file;  // Store current file
    currentMidiFile = std::make_unique<juce::MidiFile>();
    
    juce::FileInputStream fileStream (file);
    if (! fileStream.openedOk() || ! currentMidiFile->readFrom (fileStream))
    {
        currentMidiFile.reset();
        DBG ("Failed to load MIDI file: " + file.getFullPathName());
        return;
    }

    // Get tempo from MIDI file - scan for tempo events
    fileTempo = 120.0; // Default tempo
    short timeFormat = currentMidiFile->getTimeFormat();
    if (timeFormat > 0)
        ticksPerQuarterNote = static_cast<double> (timeFormat);
    else
        ticksPerQuarterNote = 480.0; // Default PPQN
    
    // Try to find tempo from MIDI events
    for (int trackIndex = 0; trackIndex < currentMidiFile->getNumTracks(); ++trackIndex)
    {
        const juce::MidiMessageSequence* track = currentMidiFile->getTrack (trackIndex);
        for (int i = 0; i < track->getNumEvents(); ++i)
        {
            const juce::MidiMessage& msg = track->getEventPointer (i)->message;
            if (msg.isTempoMetaEvent())
            {
                fileTempo = msg.getTempoSecondsPerQuarterNote() > 0.0 
                    ? 60.0 / msg.getTempoSecondsPerQuarterNote() 
                    : 120.0;
                break; // Use first tempo found
            }
        }
        if (fileTempo != 120.0) break;
    }

    int numTracks = currentMidiFile->getNumTracks();
    midiTracks.resize (numTracks);
    for (int i = 0; i < numTracks; ++i)
    {
        midiTracks[i] = *currentMidiFile->getTrack (i);
    }

    DBG ("Loaded MIDI file with " + juce::String (numTracks) + " tracks, tempo " + juce::String (fileTempo));
}

void MidiFartSnifferProcessor::startPlayback()
{
    isPlaying = true;
    currentTick = 0;
}

void MidiFartSnifferProcessor::stopPlayback()
{
    isPlaying = false;
}

void MidiFartSnifferProcessor::setLooping (bool loop)
{
    shouldLoop = loop;
}

bool MidiFartSnifferProcessor::getIsPlaying() const
{
    return isPlaying;
}

double MidiFartSnifferProcessor::getFileTempo() const
{
    return fileTempo;
}

int64_t MidiFartSnifferProcessor::getMaxTick() const
{
    int64_t maxTick = 0;
    for (const auto& track : midiTracks)
    {
        if (track.getNumEvents() > 0)
        {
            auto lastEvent = track.getEventPointer (track.getNumEvents() - 1);
            if (lastEvent != nullptr)
            {
                int64_t endTime = static_cast<int64_t> (lastEvent->message.getTimeStamp());
                maxTick = std::max (maxTick, endTime);
            }
        }
    }
    return maxTick;
}

double MidiFartSnifferProcessor::getPlaybackPosition() const
{
    int64_t maxTick = getMaxTick();
    if (maxTick > 0)
        return static_cast<double> (currentTick) / static_cast<double> (maxTick);
    return 0.0;
}

void MidiFartSnifferProcessor::addToFavorites (const juce::File& file)
{
    juce::String filePath = file.getFullPathName();
    if (! favoriteFiles.contains (filePath))
    {
        favoriteFiles.add (filePath);
    }
}

void MidiFartSnifferProcessor::removeFromFavorites (const juce::File& file)
{
    juce::String filePath = file.getFullPathName();
    favoriteFiles.removeString (filePath);
}

bool MidiFartSnifferProcessor::isFavorite (const juce::File& file) const
{
    return favoriteFiles.contains (file.getFullPathName());
}

juce::StringArray MidiFartSnifferProcessor::getFavorites() const
{
    return favoriteFiles;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MidiFartSnifferProcessor();
}
