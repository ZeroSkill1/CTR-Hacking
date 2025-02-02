# Snack World Trejareres Save Tool

Snack World Trejarers is a Japanese 3DS title which uses a distinct save data encryption/obfuscation technique to prevent save editing.

The following security mechanisms are used to prevent save data editing:
- An in-house xorshift-like algorithm which is used to obfuscate save files
- AES-128-CCM encryption for certain files in the save data
- CRC32 validation of all save files
- Usage of Secure Values to prevent tampering with the save data

The tool included here is able to fully decrypt and deobfuscate all save data files, as well as reobfuscating and reencrypting them.

To compile:

`g++ -o sw-save-tool sw-save-tool.cc -lssl -lcrypto -lz`

To use:
- Extract the extdata for the game.
- Run `sw-save-tool encrypt/decrypt inplace/separate head.sw [one or more gameN.sw files...]`
    - `encrypt` / `decrypt`: self-explanatory.
    - `inplace`: modifies the input files in place, `separate`: saves the result of the encryption/decryption in the same folder as the input files, prefixed with `dec_` or `enc_` respectively.
