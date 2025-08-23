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

https://github.com/user-attachments/assets/512e47ae-9f1d-466a-aa03-9962cbb43b20

<img width="1920" height="1080" alt="BudgetGB" src="https://github.com/user-attachments/assets/245432fb-c484-479c-88c1-af375fb5fc43" />

<img width="973" height="463" alt="zeldaLinksAwakening" src="https://github.com/user-attachments/assets/00e2a647-3664-47ff-9222-f840849bc9b4" />

<img width="641" height="606" alt="pokemonRed" src="https://github.com/user-attachments/assets/361aa7db-8649-4185-831b-6ee5335b5904" />


## Build/Install

Build with CMake using your prefered generator.

I have only tested BudgetGB on windows and mac, but it should work on linux with some minor
changes. All dependencies are included inside the vendor directory as either submodules or
from source.

``` bash
git clone --recurse-submodules https://github.com/AndyCChen/Budget-GB.git
```

```bash
cmake  -S ./ -B out 
```

Build files will be outputed into `out/`, use your prefered build system
to build and compile such as make or msbuild. Remember to set the working directory
to the project root for whatever IDE or text editor you are using so files from `resource/`
can be loaded. Or just copy paste `resource/` into the same directory as the outputed executable.

## Options

You can also provide the following cmake switches.

Use directX11 renderer when on windows, default is opengl.
```bash
cmake -DUSE_DX11_ON_WINDOWS=ON ...
```
Toggle Imgui and Implot demo windows
```bash
cmake -DENABLE_IMGUI_DEMO=ON ...
```

## Controls

`W` - Up  

`A` - Left  

`S` - Down  

`D` - Right 

`Q` - Start  

`E` - Select  

`K` - B  

`L` - A  
