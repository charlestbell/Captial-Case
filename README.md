# Capital Case

A tiny portable Windows app: type or paste text, press **Convert**, and the output box shows it in Capital Case. **Copy to clipboard** copies the result.

## Running
Download [`CapitalCase.exe`](CapitalCase.exe) (click **Download raw file** on GitHub) and double-click it. No install, no runtime, no extra files (64-bit Windows 7 or later). It uses only DLLs that ship with Windows.

Because the exe isn't code-signed, Windows SmartScreen may say "Windows protected your PC" the first time. Click **More info → Run anyway**.

## Conversion rule
The first letter of every word is upper-cased and the rest lower-cased: `hELLO wORLD` → `Hello World`. Hyphens split words (`Rock-And-Roll`), apostrophes don't (`Don't`). Line breaks and punctuation are kept. Note that acronyms get lower-cased too (`NASA` → `Nasa`).

## Building from source
Plain C against the Win32 API. With MinGW-w64 installed: `sh src/build.sh`.
Logic tests: `gcc src/test_capcase.c -o t && ./t`.
