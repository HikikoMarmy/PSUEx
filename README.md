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
