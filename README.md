# SkyMM-NX

SkyMM-NX is a simple mod manager that runs on your Switch and provides an easy way to toggle mods on
and off.

SkyMM will attempt to discover all mods present in Skyrim's ROMFS on the SD card and present them through its interface.
Through the interface, you can toggle mods on or off, or change the load order by holding `Y`. Note that the load order
for pure replacement mods (lacking an ESP) will not be preserved when the respective mods are disabled.

When the save function is invoked, the INI and `Plugins` files will be modified accordingly and saved to the SD card.

### Changes Made To Original Work
1. Change all suffixes to be single letter only. 
- For example, `EnaiRim - Textures` should now be formatted as `EnaiRim - T`.

- The skyrim.ini file has a line limit of 1024 characters, which can be hit very quickly if we use the lengthy original suffixes scheme on NexusMods (I hit it around 16 mods, even with succinct base modnames).

- This change hence aims to remove all of that unnecessary character bloat from the `skyrim.ini` configuration file so that more mods can be loaded.

2. Very small under-the-hood fixes

### Naming Scheme (updated)

Currently, the app requires that all mods follow a standard naming scheme:

- All suffixes in filenames are to be truncated to one-letter only
  - `Mod - Animations.bsa` to be renamed `Mod - A.bsa`
  - `Mod - Meshes.bsa` to be renamed `Mod - M.bsa`
  - `Mod - Sounds.bsa` to be renamed `Mod - S.bsa`
  - `Mod - Textures.bsa` to be renamed `Mod - T.bsa`
  - `Mod - Voices.bsa` to be renamed `Mod - V.bsa`
  - Additional Tip: You can further replace the basename `Mod` with something even shorter like `M` - just use common sense and make sure it doesn't conflict with the basename of another mod.

- BSA files with a suffix must use a hyphen with one space on either side between the base name and the suffix
  - Example: `Static Mesh Improvement Mod - T.bsa`
  - Note that a mod may have exactly one non-suffixed BSA file
- BSA files with an associated ESP file must match the ESP's name, not including the suffix
  - Example: `Static Mesh Improvement Mod - T.bsa` matches `Static Mesh Improvement Mod.esp`
- All BSA files for a given mod must match each other in name
  - Example: `Static Mesh Improvement Mod - T.bsa` matches `Static Mesh Improvement Mod - M.bsa`

### To-do

- Add ability to 'nickname' or give aliases to mods in-app or on PC via a `.txt` so that it's easier to identify truncated modnames (so that you don't come back to the game after 5 years and start wondering what `E.esp` does)
- Add a python or bash script to automatically to automatically detect and truncate all suffixes in a folder

### Building

SkyMM-NX depends on `devkitA64`, `libnx`, and `switch-tools` to compile. These packages are installable through
[devkitPro pacman](https://devkitpro.org/wiki/devkitPro_pacman).

Once all dependencies have been satisfied, simply run `make` in the project directory.

### License

SkyMM-NX is made available under the
[MIT License](https://github.com/caseif/SkyrimNXModManager/blob/master/LICENSE). It may be used, modified, and
distributed within the bounds of this license.
