# PSUEx

**PSUEx** is a work-in-progress DLL extension for **Phantasy Star Universe (PSU) Ver. 14.30**.  
It enhances and extends the original game by injecting new features and improvements.

> **Note:** This project is still in early development. Expect breaking changes and bugs along the way.

---

## 🔧 Features

- Hooking & extending engine behavior, as well as a number of bug fixes.
- Extended Resolution support, along with FOV correction and more.
- Floor Reader for extended item loot information.
- Camera Controls for distance, height, and rotation speed.
- UI scaling for high resolutions, and Xbox controller fixes (triggers as buttons, working out of focus, controllers plugged in after the game started).

---

## 📦 Dependencies

- [Microsoft Detours](https://github.com/microsoft/Detours)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [nlohmann/json](https://github.com/nlohmann/json)
- DirectX 9 SDK

---

## Japanese translation

PSUEx can optionally show the game's text in Japanese. 
Text the game shows, including item names, is shown in Japanese, and shop searches can be typed in Japanese.

It's off unless a `translation.txt` is in the game folder. Without it, PSUEx
works exactly as before.

### Setup

1. Put this fork's `PSUEx.dll` in the game folder.
2. Put your `translation.txt` in the game folder, next to `PSUEx.dll`.
3. Start the game.

`translation.log` in the game folder shows how many lines were read, and any
lines that couldn't be read.

The file is read again when you save it, so you can edit it while the game is
running.

### Shop search

Type an item's name in Japanese in the shop search, whole or just the start
(モノメ for モノメイト). It's sent to the server in English, using the
translation file, so you get the usual list of shops. Names typed in English
are searched as usual.

### The translation file

`translation.txt` is a UTF-8 text file with one line per translation:

```text
# Lines starting with # are comments.
Monomate => モノメイト
Lobby menu => ロビーメニュー
```

- The English must match the game's text exactly, including case and spaces.
- `{}` matches any text, such as a number or a name, and puts it in the
  translation in the same place:
  ```text
  {} has\ninvited you to join a party. => {}から\nパーティーに招待されました。
  [B] {} => 基板/{}
  ```
  Text matched by `{}` is translated too, if it has its own line (`[B] Monomate`
  shows as `基板/モノメイト`).
- `{1}`, `{2}`, ... in the translation pick a `{}` by number, for a different
  word order:
  ```text
  Partner card : {}/{} => {2}枚中{1}枚
  ```
- `\n` is a line break, `\t` a tab, and `\u{f805}` a character by its code
  (for the game's color and formatting codes). Write `\\`, `\{` and `\}` for a
  `\`, `{` or `}`.
- Exact lines win over `{}` lines. `{}` lines are tried from the top of the
  file down, so put the more specific ones first.

---

## UI scaling and controller settings

These are off by default. Turn them on in the `[custom]` section of
`PsuIlluminus.ini` (in `%LOCALAPPDATA%\SEGA\PHANTASY STAR UNIVERSE Illuminus\`),
the same file the launcher writes its options to:

```ini
[custom]
UI_SCALE=1.5
TRIGGER_FIX_MOD=1
LEFT_TRIGGER_BUTTON=10
RIGHT_TRIGGER_BUTTON=11
TRIGGER_PULL=30
CONTROLLER_BACKGROUND_MOD=1
CONTROLLER_HOTPLUG_MOD=1
```

| Setting | Default | What it does |
|---|---|---|
| `UI_SCALE` | `1` (off) | Draws the game's UI larger at high resolutions, 1 to 8, decimals allowed. 1.5 at 1920x1080, 2 at 2560x1440 or 3 at 3840x2160 draw it as large as at 1280x720. Limited so the UI still has at least 640x480 to lay out on. |
| `TRIGGER_FIX_MOD` | `0` | `1` makes an Xbox controller's triggers press buttons you can bind in the game's controller settings. Through DirectInput both triggers share one axis, so the game can't bind them otherwise. |
| `LEFT_TRIGGER_BUTTON`, `RIGHT_TRIGGER_BUTTON` | `10`, `11` | The buttons the triggers press, numbered as the controller settings show them (`BUTTON 10`). |
| `TRIGGER_PULL` | `30` | How far a trigger must be pulled to count, 1 to 255. |
| `CONTROLLER_BACKGROUND_MOD` | `0` | `1` keeps the controller working while the game window isn't in focus. |
| `CONTROLLER_HOTPLUG_MOD` | `0` | `1` sets up a controller plugged in (or turned on) after the game started. The game only looks for controllers at startup, so otherwise one connected later is never seen. Up to the game's two controllers. |

Restart the game after changing them.

**For developers.** `PSUDetour/Hook_UIScale.hpp` hooks the game's layout-size
query (`0x00777160`) so the UI lays itself out on a screen `UI_SCALE` times
smaller, then scales the game's pre-transformed (`XYZRHW`) `DrawPrimitiveUP`
and `DrawIndexedPrimitiveUP` vertices back up. The calls that size render
targets and the camera are pointed back at the real size, and full-screen
draws, small render targets, the bloom pass and draws not made by `PSUC.exe`
(other mods' overlays) are left alone. `PSUDetour/Hook_ControllerTriggers.hpp`
hooks `DirectInput8Create`, then `CreateDevice` and `GetDeviceState`, reads
the triggers of pads with `IG_` in their device path through XInput, and sets
the chosen buttons. `PSUMemory/Patch/ControllerBackgroundPatch.hpp` changes the
controller's cooperative level from foreground to background.
`PSUDetour/Hook_ControllerHotplug.hpp` hooks the game's per-frame controller
poll (`0x00786C90`); while a controller slot is free and XInput has more pads
than the game has set up, it lists the attached controllers as the game does
at startup and sets each new one up with the game's own slot setup
(`0x00786E80`). Each checks the
game's code before changing it and stays off if it doesn't match.
