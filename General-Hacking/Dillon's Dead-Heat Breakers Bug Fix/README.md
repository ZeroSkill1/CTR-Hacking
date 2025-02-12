# Dillon's Dead-Heat Breakers Bug Fix

Dillon's Dead-Heat Breakers (US) (00040000001CF300) has a critical bug that occurs when trying to enter certain sections in the game when there are pending friend requests ("provisionally registered friends", meaning when you have added friends but they haven't yet added you back). 

Due to the nature of the friends system on the 3DS, Mii data for these pending friend requests are blanked out until the friend accepts your friend request. Once accepted, the friends servers send the friend's Mii to your console.

The game does not perform any checks to make sure the gotten Mii data is valid, and as such, blank Mii data from these pending friends is assumed to always be valid. Because the game uses Mii data from both the local Miis (the ones in the Mii Maker) and friend Miis to create the in-game "gunners," this bug will cause a crash when the 3D models of the gunner characters areloaded, effectively preventing a player with pending friend requests from progressing in the game until the pending friend requests are deleted.

This patch aims to provide a solution for this issue by installing custom code that checks for blank Mii data before handing the Mii data off to be used for creating the game character models. If blank Mii data is found, it will be overwritten with a fallback Mii included within the patch, enabling players to continue progressing in the game even with pending friend requests.

**NOTE: This patch is exclusively meant for the US version of the game. It is currently unknown whether or not the different regional releases of this game also have this bug.**

How to use: 

1. Download [code.ips](./code.ips) and place it at `sd:/luma/titles/00040000001CF300/code.ips`.
2. Make sure Luma3DS game patching is enabled.
3. The game should now work as expected.
