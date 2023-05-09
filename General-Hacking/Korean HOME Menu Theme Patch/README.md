# Korean HOME Menu

A patch for the Korean version of the Nintendo 3DS HOME menu to enable the usage of themes and the HOME menu layout saving feature. By default, the KOR region does not have support for themes or saving layouts. Nevertheless, all the required code is still present in the code binary. The patch will fix the stock code not accounting for the KOR region by ensuring the extdata ID is set. An additional patched layout file in the RomFS will make the hidden buttons visible again.

Notes:
- I used the extdata ID 000002CF.
- This patch is known to work on the latest version of the KOR HOME menu at the time of writing this (v15361).

How to use:

- Enable Luma3DS game patching.
- Copy [code.ips](./code.ips) to `sd:/luma/titles/000400300000A902/code.ips`.
- Copy [petit_LZ.bin](./petit_LZ.bin) to `sd:/luma/titles/000400300000A902/romfs/petit_LZ.bin`.
- Turn on your system.
- Upon opening the menu on the top left corner of the HOME menu, the theme menu button and HOME menu layout button should reappear and be functional.

Credit goes to [@cooolgamer](https://github.com/cooolgamer) for patching `petit_LZ.bin`.
