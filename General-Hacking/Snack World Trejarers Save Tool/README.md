# Snack World Trejareres Save Tool

Snack World Trejarers is a Japanese 3DS title which uses a distinct save data encryption/obfuscation technique to prevent save editing.

The following security mechanisms are used to prevent save data editing:
- An in-house xorshift-like algorithm which is used to obfuscate save files
- AES-128-CCM encryption for certain files in the save data
- CRC32 validation of all save files
- Usage of Secure Values to prevent tampering with the save data

The tool included here is able to:
- Fully decrypt and deobfuscate all save data files
- (Coming soon: Fully encrypt and reobfuscate all save data files)

To compile:

`g++ -o decrypt_tool decrypt.cc -lssl -lcrypto -lz`

To use:
- Extract the extdata for the game.
- Run `decrypt_tool head.sw [one or more gameN.sw files...]`
- The tool will save decrypted files in the same location as the source files, each prefixed with `dec_`.
