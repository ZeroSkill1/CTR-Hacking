# Home Menu Patches

## KOR Home Menu Theme + Layout Saving Patch

NOTE: This patch also enables the usage of badges. Details further below.

A patch for the Korean version of the Nintendo 3DS HOME menu to enable the usage of themes and the HOME menu layout saving feature. By default, the KOR region does not have support for themes or saving layouts. Nevertheless, all the required code is still present in the code binary. The patch will fix the stock code not accounting for the KOR region by ensuring the extdata ID is set. An additional patched layout file in the RomFS will make the hidden buttons visible again.

Notes:
- I used the extdata ID 000002CF.
- This patch is known to work on the latest version of the KOR HOME menu at the time of writing this (v15361).
- As the localization files do not include translations for the text related to themes and layout saving, some UI elements may appear without text. This does not affect functionality, however.

How to use:

- Enable Luma3DS game patching.
- Copy [code.ips](./KOR/code.ips) to `sd:/luma/titles/000400300000A902/code.ips`.
- Copy [petit_LZ.bin](./KOR/petit_LZ.bin) to `sd:/luma/titles/000400300000A902/romfs/petit_LZ.bin`.
- Turn on your system.
- Upon opening the menu on the top left corner of the HOME menu, the theme menu button and HOME menu layout button should reappear and be functional.

Credit goes to [@cooolgamer](https://github.com/cooolgamer) for patching `petit_LZ.bin`.

## Badge NNID Patch

The HOME Menu uses the Nintendo Network ID (NNID)'s PrincipalId and stores it inside the badge save files. Due to this, it is neither possible to create badge save data without a Nintendo Network ID, nor is it possible to restore the badge data from one console onto another without manual modifications.

This patch completely removes this requirement by bypassing all checks related to NNIDs and badge save data creation and reading, allowing the creation of badge save data and usage of foregin consoles' badge data as-is. This is especially important for the KOR region as there is no way to link an NNID on that region.

Notes:
- Patches are made for the latest versions of the respective regions' HOME Menus (JPN, EUR, USA, KOR).

How to use:

- Enable Luma3DS game patching.
- Copy `code.ips` ([JPN](./JPN/code.ips) [EUR](./EUR/code.ips) [USA](./USA/code.ips) [KOR (also includes Themes + Layout saving, see above)](./KOR/code.ips)) to:
    - `sd:/luma/titles/0004003000008202/code.ips` for JPN
    - `sd:/luma/titles/0004003000009802/code.ips` for EUR
    - `sd:/luma/titles/0004003000008F02/code.ips` for USA
    - `sd:/luma/titles/000400300000A902/code.ips` for KOR
- Badge functionality should now be working without NNID checks!
