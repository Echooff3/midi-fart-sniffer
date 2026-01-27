# Build Failure Fix - Submodule Issue

## Problem
The Windows CI build was failing with the following error:
```
fatal: No url found for submodule path 'build/_deps/juce-src' in .gitmodules
```

## Root Cause
When build artifacts were initially committed to the repository, the `build/_deps/juce-src` directory (which is a git clone of the JUCE framework) was accidentally registered as a git submodule (file mode 160000). This happened because:

1. CMake's FetchContent downloaded JUCE as a git repository into `build/_deps/juce-src`
2. The entire build directory was committed before `.gitignore` was added
3. Git detected the nested git repository and registered it as a submodule
4. However, no `.gitmodules` file was created with the submodule URL

## Impact
When GitHub Actions tried to checkout the code with `submodules: recursive`, it attempted to initialize the submodule but failed because there was no URL configured in `.gitmodules`.

## Fix Applied
1. Removed the problematic submodule entry: `build/_deps/juce-src`
2. Removed all 233 build artifact files from git tracking
3. Verified `.gitignore` properly excludes the `build/` directory

## Result
- Build artifacts are no longer tracked in git
- CI will generate fresh builds on each run (as intended)
- No submodule conflicts
- Future builds will work correctly on Windows, Linux, and macOS

## Files Changed
- Deleted 233 files from `build/` directory
- `.gitignore` already in place to prevent future issues

## Verification
```bash
# Check that no build files are tracked
git ls-files | grep "^build/" 
# Should return nothing (success)

# Verify gitignore
cat .gitignore | grep build
# Should show: build/
```
