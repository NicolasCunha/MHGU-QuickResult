# Quick Result

A [Forge](https://github.com/Fexty12573/forge) plugin for [MHGU Recomp](https://github.com/morerokk/MHGURecomp)
(Monster Hunter Generations Ultimate 1.4.0) that **skips the "Quest Complete" screen** at the end of a quest, so the
game goes straight to the rewards.

[Leia em português](README.pt-BR.md)

- The Quest Complete screen normally holds you for about 30 seconds. With Quick Result it is gone as soon as it starts.
- Optional: also skip the "return to village" countdown right after the clear.
- Both switches are in Forge's menu (press **Insert** in game, then open **Quick Result**). The Quest Complete skip is
  on by default and the countdown skip is off. The switches reset each time the game starts.

## How it works

After a quest is cleared, the game goes through these quest states (byte `+0x50` of the quest object):

| State | What you see | How the game leaves it |
|---|---|---|
| 3 | Quest cleared, "return to village" countdown | a countdown (u16) reaches 0 |
| 4 | **The Quest Complete screen** | the **same countdown**, restarted at 899, reaches 0 |
| then | Loading, then the result screen | the result screen saves the clear and shows the rewards |

Quick Result sets that countdown to 0 while the quest is in state 4, which is the game's own way out of it. Nothing
else is skipped: the result screen, where the game saves the clear, runs as usual. The countdown is only touched while
it holds a real countdown value (at most 1200); the values the game uses as markers while loading are left alone.

The addresses are in [`src/quickresult.c`](src/quickresult.c). They were found with the research build in
[`research/diag.c`](research/diag.c), which only logs the quest-end flow to `log.txt` (`sh build.sh diag`).

## Install

1. Download `QuickResult-<version>.zip` from [Releases](../../releases), or build it (see below).
2. Extract it into the `mods` folder next to `mhgurecomp.exe`, so you get `mods/QuickResult/romfs/nativeNX/plugins/quickresult.nro`.
3. Start the game. `log.txt` should have the line `[QuickResult] 1.1.0 ready: ...`, and after each quest
   `[QuickResult] Quest Complete screen skipped (...)`.

To uninstall, delete `mods/QuickResult`.

### Requirements

- MHGU Recomp with Forge (Forge 1.5.1 is built into the recomp).
- The game version the recomp uses: MHGU **1.4.0**.

### Works well with

- **SkipEndQuestTime**: hold L + R + Right to skip the return countdown on demand (or turn on Quick Result's own option).
- [**Field QoL**](https://github.com/mhvuze/mhgu-field-qol): "Send all to Item Box & Exit" on the reward screen.

### Known limits

- Tested in single player (offline). In online multiplayer the other hunters may still be on their Quest Complete
  screen. If you see problems online, turn the skip off in the Forge menu.
- Tested on Windows. It should work on any platform the recomp runs on, but that hasn't been tested.

## Build

You need:

| Dependency | Windows | Linux | macOS |
|---|---|---|---|
| A POSIX shell | [Git for Windows](https://git-scm.com/download/win) (Git Bash) | built in | built in |
| Arm GNU Toolchain (`arm-none-eabi-gcc`) | `winget install Arm.ArmGnuToolchain` | `sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi` | `brew install arm-none-eabi-gcc` |
| Python 3 | `winget install Python.Python.3.12` | `sudo apt install python3` | `brew install python` |

devkitPro is **not** needed.

Then, from the repository folder (Git Bash on Windows):

```sh
sh build.sh                                # out/quickresult.nro
sh build.sh release                        # dist/QuickResult-<version>.zip, ready to upload as a release
RECOMP=/d/MHGURecomp sh build.sh install   # build and copy into <recomp>/mods/QuickResult
sh build.sh diag                           # out/quickresult_diag.nro, the research build (logs only)
```

On Windows the script finds the Arm toolchain in its default install folder and Python in the python.org install
folder, even when they are not on `PATH`. Set `CROSS=<prefix>` or `PYTHON=<command>` to use other ones.

## Repository layout

```
src/quickresult.c     the plugin
src/forge.h           the part of Forge's plugin API used here
src/start.S           NRO entry and MOD0 header
build/nro.ld          linker script (NRO segment layout)
build/elf2nro.py      turns the linked ELF into an .nro
research/diag.c       logging build used to find the addresses
build.sh              build, release and install script
```

## Credits

- [Forge](https://github.com/Fexty12573/forge) by Fexty12573 and the
  [MHGU Recomp](https://github.com/morerokk/MHGURecomp) project. The recomp's generated C code made it possible to
  find the result screen's state machine.
- `src/start.S`, `build/nro.ld` and `build/elf2nro.py` come from lick-mee's
  [mhgu-gathering-map](https://github.com/lick-mee/mhgu-gathering-map). `src/forge.h` and the build setup come from
  Vuze's [mhgu-field-qol](https://github.com/mhvuze/mhgu-field-qol). Both are MIT, see
  [THIRD_PARTY_NOTICES](THIRD_PARTY_NOTICES).
- SkipEndQuestTime, whose countdown pointer was the starting point.

## License

MIT, see [LICENSE](LICENSE).
