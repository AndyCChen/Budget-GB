#  Gameboy Emulator

Gameboy emulator written in C++.

- [x] sm83 cpu emulation with single intruction stepping and logging
- [x] ppu emulation for pixel rendering
- [x] apu emulation for audio output
- [x] tile view to display pixel tiles in vram
- [x] audio waveform visualization 
- [x] cartridge mapper implementations for MBC1, MBC2, MBC3
- [x] renderering layer for both DirectX11 and Opengl
- [x] Option to provide dmg bootrom on cartridge startup
- [x] [zfast-lcd](https://github.com/libretro/slang-shaders/blob/master/handheld/shaders/zfast_lcd.slang) shader from retroarch

## Build/Install

Build with CMake.

I have only tested BudgetGB on windows and mac, but it should work on linux with some minor
changes. All dependencies are included inside the vendor directory as either submodules or
from source.

``` bash
git clone --recurse-submodules https://github.com/AndyCChen/Budget-GB.git
```

```bash
cmake  -S ./ -B out 
```

Build files will be outputed into out/, use your prefered build system
to build and compile such as make or msbuild.

## cmake options

You can also provide the following cmake switches.

Use dx11 renderer when on windows, default is OpenGl.
```bash
cmake -DUSE_DX11_ON_WINDOWS=ON ...
```
Toggle Imgui and Implot demo windows
```bash
cmake -DENABLE_IMGUI_DEMO=ON ...
```

