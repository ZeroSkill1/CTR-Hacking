# Chibi-Robo! Photo Finder Outfit Unlock

Chibi-Robo! Photo Finder (US/EU) for the Nintendo 3DS included outfits for the in-game character. These outfits were meant to be unlocked through an in-game contest relying on SpotPass to unlock the outfits for winners.

Since the contest has concluded years ago, there is no way to unlock these outfits except for save editing.

I have examined this game and found that most if not all of the game is run in a custom scripting engine, which is almost like emulating a whole CPU! Internally, the game refers to this pseudo-CPU as `n2::VirtualMachine`. These binary "scripts" are .nb files and have the header magic N2BC. I decided to look further into reverse engineering the format and was able to write a full disassembler for it. The code for the disassembler is included.

I was not aware of the save editing method of unlocking these outfits, and therefore I used a different technique to achieve the same result: modifying the script responsible for checking whether the player has any of the outfits.

How to use:

1. Download the .ips file corresponding to the region of the game you have: [USA](./US_title.nb.ips) | [EUR](./EU_title.nb.ips)
    - To avoid sharing the actual game files, IPS patches are used instead.
2. Using GodMode9 or another method, extract the following file from the game's RomFS: `romfs:/hio/script/title.nb`.
3. Using an IPS patcher, apply the IPS patch you downloaded to the `title.nb` file.
    - You can also use a website like [RomPatcher.js](https://www.marcrobledo.com/RomPatcher.js) to do this.
4. Rename the patched file to `title.nb` if it is not named like that already.
5. Place this patched `title.nb` file into the following folder depending on your game's region (create folders where needed):
    - `sd:/luma/titles/0004000000107C00/romfs/hio/script/title.nb` for USA;
    - `sd:/luma/titles/00040000000F7600/romfs/hio/script/title.nb` for EUR.
6. Make sure Luma3DS game patching is enabled.
7. Upon launching the game and heading into `Options`, you should have an `Outfits` menu entry where you can choose outfits!