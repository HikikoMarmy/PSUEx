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

---

## 📦 Dependencies

- [Microsoft Detours](https://github.com/microsoft/Detours)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [nlohmann/json](https://github.com/nlohmann/json)
- DirectX 9 SDK
  ## Japanese translation

PSUEx can show optional Japanese translation to PSUEx. Text the game shows,
including item names, is shown in Japanese, and shop searches can be typed in
Japanese.

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

### How it works (for developers)

The code is in two places:

- `PSUTranslation/Translation.h`, `Translation.cpp`: the translation file
  parser and lookup (`Translations`), and the `Translation` namespace that
  loads the file, caches results and writes `translation.log`.
- `PSUDetour/Hook_TextDraw.hpp`: the hooks, installed by `TextDraw::Hook()`
  from `DetoursHookAll()` in `dllmain.cpp`.

**Startup.** `TextDraw::Hook()` returns at once if `translation.txt` isn't in
the game folder, so nothing is hooked. Otherwise it compares the first bytes
of each routine it hooks with the expected code, so a different game version
isn't patched in the wrong place. If the drawing routine doesn't match, the
translation stays off. If the measuring routine or `Channel::Send` doesn't
match, only that part is skipped. The log says which.

**Drawing text** (`0x004BB310`). The game draws every string through this
routine. It's a `__thiscall` taking an output pointer, a 64-byte settings
block passed by value, and one more int. It cleans `0x48` bytes off the
stack. The UTF-16 string pointer is the block's third dword (+8). The hook is
written as `__fastcall` (ECX = `this`, EDX unused) with the block as a struct,
so the stack layout matches. When the string has a translation, the hook
swaps that pointer for the translation's, then calls the original.

**Measuring text** (`0x004BCAC0`). A `__cdecl` routine (output size, string,
two more values) the game uses to size boxes, such as the label over an item
on the ground. The hook measures the translation instead, so boxes fit the
text that's drawn.

**Shop search.** The game has an unused send-hook pointer at `0x00A8E0EC`:
`Channel::Send` (`0x0079B6D0`) calls it, if set, with each outgoing packet
before encryption and the record checksum. The fork sets it only if it's
still null and `Channel::Send` has the expected code. The handler looks for
shop search packets (size `0x84`, opcode `0x3713`) and reads the name at
`+0x44` (UTF-16, 31 characters and a null). If the name isn't plain ASCII,
it's replaced with its English from the translation file. Since the change is
made before the checksum is added, the server accepts the packet.

**Finding the English for a search** (`Translations::EnglishFor`):

1. An exact line whose translation is the typed text. If several match, the
   shortest English is used.
2. Otherwise, the lines whose translation starts with the typed text. Their
   English names' common start is used, if it's at least 3 characters
   (モノメ gives Monomate). Case is ignored when comparing, as the server's
   search does.

**Translating a string** (`Translations::Translate`). An exact line is looked
up in a hash map. Failing that, the `{}` lines are tried in file order. Each
`{}` takes the shortest text that lets the rest of the line match, and the
last one takes what's left. Text a `{}` matched is translated by its own exact
line, if there is one.

**Caching and memory.** The game draws the same strings every frame, so
results are cached:

- Translations already made (up to 20,000) are kept in a map, with their
  buffers never freed: the game may still hold the pointer after the call.
- Strings with no translation (up to 4,096, then cleared) are remembered too,
  so the `{}` lines aren't tried again each frame.
- A mutex guards the table, since the send hook may run on another thread
  than drawing.

**Reloading.** The draw hook checks `translation.txt`'s modified time at most
once a second. When it changes, a new table is built and swapped in. The old
one is left in memory, since the game may still point into it.

**Safety.** String pointers below `0x10000` are ignored, and so are strings of
1,024 characters or more.

### Building

Build `PSUEx.sln` in Visual Studio 2022 (Release, Win32), the same as PSUEx.
