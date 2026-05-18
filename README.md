# Street Kebab Fighter

> *A 2D fighting game about settling disputes the old-fashioned way with your favorite characters from "Čaršija"*

<p align="center">
  <img src="assets/images/SKF_logo.png" alt="Street Kebab Fighter Logo" width="128"/>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/platform-Windows-blue"/>
  <img src="https://img.shields.io/badge/platform-Linux-black"/>
  <img src="https://img.shields.io/badge/license-MIT-green"/>
  <img src="https://img.shields.io/badge/language-C-darkblue"/>
</p>

> [!WARNING]
> The game is still in development.

## Features
- *1 playable character* 
- *2 playable stage* 

---

## Controls

| Action | Player 1 | Player 2 |
|---|---|---|
| Jump | **W** | **UP** |
| Move Forward, Backward  | **A, D** | **LEFT, RIGHT** |
| Crouch | **S** | **DOWN** |
| Light Attack | **F** | **I** |
| Medium Attack | **G** | **O** |
| Heavy Attack | **H** | **P** |
| Block | Backward + Light Attack |  Backward + Light Attack |

## Platform Support

| Platform | Status |
|---|---|
| Windows | ✅ Supported |
| Linux | ✅ Supported |
| Web | 🔜 Coming soon |

## Dependencies

| Dependency | Version |
|---|---|
| SDL2 | *2.32.6* |
| SDL2_image | *2.8.2* |
| SDL2_mixer | *2.8.0 (Unused)* |

## Build

The game can be built using a "nob.h" (Tscoding: [Github](https://github.com/tsoding)) script, but it only supports **gcc** or **clang** compilers. 

After installing the repo the game is built in 2 steps:
1. Compiling the script once 
2. Running nob to build the game

If you want to know why would you even compile the script for building just to, build again, go look at repository of [nob.h](https://github.com/tsoding/nob.h).

### Windows

1. Open *Cmd* or *PowerShell* and check if you are in the root folder of the repo 
(something like: ...\path\to\Street-Kebab-Fighter)
and run this command to compile the script: 
```
gcc -o nob.exe nob.c
```

2. Then run the nob script with:
```
.\nob -run
```

### Linux

> [!WARNING]
> No support for Linux static linking.

1. Open *Bash* and check if you are in the root folder of the repo 
(something like: .../path/to/Street-Kebab-Fighter)
and run this command to compile the script: 
```
gcc -o nob nob.c
```

2. Then run the nob script with:
```
./nob -run
```

If the linker spits out a error about 'undefined references' 
then you most likely dont have SDL2 dependenies installed.
Here are the install commands across the major Linux distros and package managers:

---

**Debian / Ubuntu / Mint (apt)**
```
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev
```

**Fedora / RHEL / CentOS (dnf)**
```
sudo dnf install SDL2-devel SDL2_image-devel SDL2_mixer-devel
```

**Arch / Manjaro (pacman)**
```
sudo pacman -S sdl2 sdl2_image sdl2_mixer
```

**openSUSE (zypper)**
```
sudo zypper install libSDL2-devel libSDL2_image-devel libSDL2_mixer-devel
```

**Gentoo (emerge)**
```
sudo emerge media-libs/libsdl2 media-libs/sdl2-image media-libs/sdl2-mixer
```

**NixOS / nix-shell**
```
nix-shell -p SDL2 SDL2_image SDL2_mixer
```

---

After you installed dependenies you can just run *./nob -run* to compile the game again. 

### Build options

Scrpit has a few flags like:
- "-run" - Runs the game after compilation 
- "-static" - Links the game statically instead of dynamically
- "-dbg" - Compile with debug info
- "-cc" - Change the compiler with the name of next argument

By default gcc is used but you can replace it with clang if you want.

## Roadmap

- [x] *Collision and attacking systems*
- [x] *Build script with "nob.h"*
- [x] *Linux support*
- [ ] *Asset baking* - All assets directly included into the game binary  
- [ ] *Web support*

---

## Credits

- ✏️ *Art — LomimStaklo*
- ✏️ *Jojo's Bizarre Adventure Regular — Patrick H. Lauke*

---

## License

This project is licensed under the MIT License — see [LICENSE](LICENSE) for details.
