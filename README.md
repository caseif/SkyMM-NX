# SkyMM-NX-SMM

SkyMM-NX-SMM is a homebrew app (or plugin for SimpleModManager if you change the file extension to .smm) that runs on your Switch and provides an easy way to toggle skyrim mods on
and off.

SkyMM will attempt to discover all mods present in Skyrim's ROMFS on the SD card and present them through its interface.
Through the interface, you can toggle mods on or off, or change the load order by toggling `Y`. Note that the load order
for pure replacement mods (lacking an ESP) will not be preserved when the respective mods are disabled.

When the save function is invoked, the INI and `Plugins` files will be modified accordingly and saved to the SD card.

Currently, the app requires that all mods follow a standard naming scheme:

- BSA files with a suffix must use a hyphen with one space on either side between the base name and the suffix
  - Example: `Static Mesh Improvement Mod - Textures.bsa`
  - Note that a mod may have exactly one non-suffixed BSA file
- BSA files with an associated ESP/ESM file must match the ESP/ESM's name, not including the suffix
  - Example: `Static Mesh Improvement Mod - Textures.bsa` matches `Static Mesh Improvement Mod.esp`
- All BSA files for a given mod must match each other in name
  - Example: `Static Mesh Improvement Mod - Textures.bsa` matches `Static Mesh Improvement Mod - Meshes.bsa`

### To-do

- Graceful error handling
  - I've done minimal edge testing so far, so the app probably won't respond well to most less-than-ideal
    conditions (e.g. missing or malformed files)
- INI injection support
  - Injecting INIs is easy; the hard part is disabling them in a sane way

### Docker

SkyMM-NX-SMM has a vscode devcontainer setup so you just need the vscode plugin [ms-vscode-remote.remote-containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) and open the project in vscode and it will ask you to open in docker container

### Building

Once the docker environment has been set up, simply run `make` in the project directory.

### License

SkyMM-NX-SMM is made available under the
[MIT License](https://github.com/withertech/Sky-MM-NX-SMM/blob/master/LICENSE). It may be used, modified, and
distributed within the bounds of this license.
