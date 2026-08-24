# debugmenu-imgui
Dear ImGui implementation for aap's debugmenu

## Supported games:
GTA 3 1.0 US, GTA VC 1.0, GTA SA 1.0 US

## How to use:
Download the latest release from the releases page and copy debugmenu.dll to your game folder (next to the exe). Make sure you have a plugin that uses it. 
Plugins that used aap's debugmenu are fully compatible with this as this is intended to be a replacement for it.

On first run, a `debugmenu.ini` is created next to the DLL. It lets you configure:
- The toggle hotkey (default `Ctrl+M`) and its modifiers
- Font scale
- Whether the menu window's position/size is remembered between sessions (saved to `debugmenu_imgui.ini`)

A search box at the top of the menu lets you filter entries by name across all pages.

## How to build:
Setup plugin-sdk and build using the solution in the project-files folder. Pick the `Release`/`Debug` configuration for GTA 3, `Release VC`/`Debug VC` for Vice City, or `Release SA`/`Debug SA` for San Andreas.

## Thanks to:

aap (for the original debugmenu)

ermaccer (for his d3d9 hook and ImGui implementation)

TsudaKageyu (for MinHook)

Plugin-SDK
