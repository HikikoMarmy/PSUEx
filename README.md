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

## Translation

PSUEx can optionally show the game's text in another language, such as
Japanese. Text the game shows, including item names and NPC message boxes that
type their text out letter by letter (such as the GAS counter's description),
is shown translated, and shop searches can be typed in the translation.

It's off unless a language is set up. Without one, PSUEx works exactly as
before.

### Setup

1. Put this fork's `PSUEx.dll` in the game folder.
2. Put the language's folder in a `Lang` folder in the game folder:
   ```text
   Lang\
     ja\
       ja.json        the text
       data\          the game's data files in this language (optional)
   ```
3. Set `LANGUAGE=ja` (the folder's name) in the `[custom]` section of
   `PsuIlluminus.ini` (in
   `%LOCALAPPDATA%\SEGA\PHANTASY STAR UNIVERSE Illuminus\`).
4. Start the game.

`translation.log` in the game folder shows how many lines were read, and any
that couldn't be read.

The file is read again when you save it, so you can edit it while the game is
running.

Each language is a folder of its own, so several can sit side by side; only
the one `LANGUAGE` names is used.

### Data files

Some text is in the game's own data files rather than drawn as is (menus,
the login and character screens, room and shop text, mission text). A
language's `data` folder holds copies of those files in that language, named
like the files in `DATA` (32 characters). They're loaded in place of the
originals only while that language is set, the same way as files in `Addon`.
A file in `Addon` wins over the language's copy.

### Finding text to translate

With `LOG_UNTRANSLATED=1` in the `[custom]` section of `PsuIlluminus.ini`, the
game's text that has no translation yet is written to `untranslated.txt` in
the game folder, each as a line ready to fill in: with a `LANGUAGE` set, as an
entry to paste into `exact` (`"English": "",`), else as a `translation.txt`
line (`English => `). Each string is written once, up to 1,000 per session. Only text shown after you log in is
written, so the login screen (your PSU ID) stays out of it. Restart the game
after changing the setting.

### Shop search

Type an item's name as it's translated in the shop search, whole or just the
start (モノメ for モノメイト). It's sent to the server in English, using the
translation file, so you get the usual list of shops. Names typed in English
are searched as usual.

### The language file

`Lang\<LANGUAGE>\<LANGUAGE>.json` is a UTF-8 JSON file:

```json
{
  "name": "日本語",
  "exact": {
    "Menus": {
      "Monomate": "モノメイト",
      "Lobby menu": "ロビーメニュー"
    }
  },
  "patterns": [
    [ "{} has\ninvited you to join a party.", "{}から\nパーティーに招待されました。" ],
    [ "Partner card : {}/{}", "{2}枚中{1}枚" ]
  ]
}
```

- `exact` holds the text, `"English": "translation"`, in groups of any
  name (the groups are only for finding your way around the file; lines can
  also sit in `exact` directly). The English must match the game's text
  exactly, including case and spaces. Keep each English text once: if it's
  in two groups, which translation wins isn't decided by the file's order.
- `name` is the language's own name, shown in `translation.log`.
- Text is written as JSON writes it: `\n` is a line break, `\"` a quote, and
  `\uf805` a character by its code (for the game's color and formatting
  codes).
- `patterns` hold text with `{}`, which matches any text, such as a number or
  a name, and puts it in the translation in the same place. Text matched by
  `{}` is translated too, if it has its own line (`[B] Monomate` shows as
  `基板/モノメイト` with `[ "[B] {}", "基板/{}" ]`).
- `{1}`, `{2}`, ... in a pattern's translation pick a `{}` by number, for a
  different word order. In patterns, `\\{`, `\\}` and `\\\\` (in JSON) are a
  literal `{`, `}` and `\`.
- Exact text wins over patterns. Patterns are tried from the top of the list
  down, so put the more specific ones first.

### translation.txt

With no `LANGUAGE` set, PSUEx reads `translation.txt` from the game folder
instead, an older format with one line per translation:

```text
# Lines starting with # are comments.
Monomate => モノメイト
{} has\ninvited you to join a party. => {}から\nパーティーに招待されました。
```

- `{}`, `{1}`, `{2}`, ... work as in the language file's patterns.
- `\n` is a line break, `\t` a tab, and `\u{f805}` a character by its code
  (for the game's color and formatting codes). Write `\\`, `\{` and `\}` for a
  `\`, `{` or `}`.
- Exact lines win over `{}` lines. `{}` lines are tried from the top of the
  file down, so put the more specific ones first.
