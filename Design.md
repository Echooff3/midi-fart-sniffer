# Midi fart sniffer
You are tasked with creating a simple VST plugin that plays MIDI files. The plugin should allow users to browse for MIDI files and play them back through a VST instrument. You are a master of JUCE, a widely used framework for audio applications, and will use it to implement this functionality. The final product should be a VST plugin that can be used in any DAW that supports VST plugins.

## Interface
It should have a browse button to open a folder

Then there should be a listbox underneath of the midi files in that folder.

If I click on a midi file in the listbox it should start playing that midi track. Clicking the same file again should stop playback. It can gleam the tempo from the actual project.

## Build
This project was setup using JUCE. The documentation for the api is here http://docs.juce.com/master/index.html

This tutorial should get you most of the way there.
https://juce.com/tutorials/tutorial_midi_message/

Be sure to include any libraries/modules from JUICE that may be required.
That is located here: C:\JUCE