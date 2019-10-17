# SkyMM-NX

SkyMM-NX is a simple mod manager that runs on your Switch and provides an easy way to toggle mods on
and off.

SkyMM will attempt to discover all mods present in Skyrim's ROMFS on the SD card and present them through its interface.
When a mod is toggled on or off, the INI and `Plugins` files will be modified accordingly.

Currently, the program requires that all mods follow a standard naming scheme:

- BSA files with a suffix must use a hyphen with one space on either side between the base name and the suffix
  - Example: `Static Mesh Improvement Mod - Textures.bsa`
  - Note that a mod may have exactly one non-suffixed BSA file
- BSA files with an associated ESP file must match the ESP's name, not including the suffix
  - Example: `Static Mesh Improvement Mod - Textures.bsa` matches `Static Mesh Improvement Mod.esp`
- All BSA files for a given mod must match each other in name
  - Example: `Static Mesh Improvement Mod - Textures.bsa` matches `Static Mesh Improvement Mod - Meshes.bsa`

### Building

SkyMM-NX depends on `devkitA64` and `libnx` to compile. These packages are installable through
[devkitPro pacman](https://devkitpro.org/wiki/devkitPro_pacman).

Once all dependencies have been satisfied, simply run `make` in the project directory.

### License

SkyMM-NX is made available under the
[MIT License](https://github.com/caseif/SkyrimNXModManager/blob/master/LICENSE). It may be used, modified, and
distributed within the bounds of this license.
