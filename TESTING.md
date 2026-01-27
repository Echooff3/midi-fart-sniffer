# Testing Guide for New Features

## Prerequisites
- Built VST3 plugin from the build directory
- A DAW that supports VST3 plugins (e.g., Reaper, Ableton Live, FL Studio)
- A folder with MIDI files for testing

## Feature 1: Auto-play Checkbox Testing

### Test Steps
1. Load the MidiFartSniffer VST3 plugin in your DAW
2. Navigate to a folder containing MIDI files using the file browser
3. **Without auto-play enabled:**
   - Click on a MIDI file
   - Verify the file loads but does NOT start playing automatically
   - Click the "Play" button to start playback
   
4. **Enable auto-play:**
   - Check the "Auto-play" checkbox in the right panel
   - Click on a different MIDI file
   - ✅ **Expected:** The file should immediately start playing
   
5. **Toggle playback by clicking:**
   - While a file is playing (with auto-play enabled)
   - Click the same file again in the browser
   - ✅ **Expected:** Playback should stop
   - Click the file again
   - ✅ **Expected:** Playback should start again
   
6. **Stop button still works:**
   - While a file is playing
   - Click the "Stop" button
   - ✅ **Expected:** Playback should stop

7. **State persistence:**
   - With auto-play enabled, close the plugin
   - Reopen the plugin
   - ✅ **Expected:** Auto-play checkbox should still be checked

## Feature 2: Favorites List Testing

### Test Steps
1. Load the MidiFartSniffer VST3 plugin in your DAW
2. Navigate to a folder containing MIDI files

3. **Adding a favorite:**
   - Click on a MIDI file to select it
   - Verify the button shows "☆ Favorite"
   - Click the "☆ Favorite" button
   - ✅ **Expected:** 
     - Button changes to "★ Unfavorite"
     - File appears in the "Favorites:" list at the bottom of the right panel
     
4. **Adding multiple favorites:**
   - Click on a different MIDI file
   - Click the "☆ Favorite" button
   - ✅ **Expected:** This file also appears in the favorites list
   - Repeat with 2-3 more files
   
5. **Playing from favorites:**
   - Click on a file in the favorites list
   - ✅ **Expected:** 
     - The file loads
     - Playback starts immediately (regardless of auto-play setting)
     
6. **Removing a favorite:**
   - With a favorite file selected
   - Verify the button shows "★ Unfavorite"
   - Click the "★ Unfavorite" button
   - ✅ **Expected:**
     - Button changes to "☆ Favorite"
     - File disappears from the favorites list
     
7. **State persistence:**
   - Add 2-3 files to favorites
   - Close the plugin
   - Reopen the plugin
   - ✅ **Expected:** All favorite files should still appear in the favorites list
   - Click on one of them
   - ✅ **Expected:** It should load and play

## UI Layout Verification

The right panel should display (from top to bottom):
1. Play button | Stop button
2. Loop button | Sync to Host button
3. Auto-play checkbox
4. ★ Favorite button
5. Position slider
6. File name label
7. Status label
8. Tempo label
9. "Favorites:" label
10. Favorites list (with all favorite files)

## Edge Cases to Test

1. **Favorite a file, then delete it from disk:**
   - The file should remain in the favorites list
   - Clicking it should handle the missing file gracefully

2. **Click same file in browser and favorites:**
   - Should toggle playback correctly

3. **Multiple DAW sessions:**
   - Favorites and auto-play state should be independent per plugin instance

## Known Behavior

- Clicking a favorite file ALWAYS starts playback, even if auto-play is disabled
- Clicking the same file again (browser or favorites) stops playback
- The Stop button can be used at any time to stop playback
- Favorites are stored as absolute file paths
