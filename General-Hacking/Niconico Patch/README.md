# Niconico Patch

Nicnico requires NEX game server authentication in order to function.

Furthermore, if an NNID is linked, the app will force NEX authentication through the NNID.

The app's last update also disables StreetPass (surema) functionality by forcefully disabling the menu option and removing the StreetPass toggle in the app's settings.

This patch does the following:

- Bypassing NEX game server authentication by tricking the app into thinking it succeeded;
- Allowing usage of the legacy login by making the app think no NNID is linked even if one is;
- Reenabling the StreetPass (surema) menu option and replacing the useless "About App" entry in the settings screen with the previously removed StreetPass toggle.

Niconico (0004000000109800) with version v20896 is required.

To install the patch, place `code.ips` into `sd:/luma/titles/0004000000109800/code.ips`.

Once installed, the patch should work out of the box.

NOTE: The re-added StreetPass toggle may not accurately display whether or not StreetPass is enabled for Niconico. To make sure it's enabled, tap on the toggle until you see a loading circle that soon finishes. Then, open System Settings -> Data Management -> StreetPass Management and see if Niconico is present in the grid. If it isn't, try this process again.
