#include <patch.hh>

int main(int argc, char* argv[])
{
	gfxInitDefault();
	amInit();
	fsInit();

	consoleInit(GFX_TOP, NULL);

	printf(
		"              - Rusty's Real DeaLC -              \n"
		"  https://github.com/ZeroSkill1/CTR-AOC-Patching  \n\n");

	printf("Patching...\n\n");

	Result res = DLC::RustysRealDealBaseball::Patch();

	switch (res)
	{
		case 0:
			printf("SUCCESS: All installed versions of\nRusty's Real Deal Baseball have been patched!\n\n");
			break;
		case TITLE_VERSION_MISMATCH:
			printf(
				"ERROR: Title version mismatch - one or more\n"
				"versions of the game/DLC are outdated.\n"
				"Please make sure you have the latest version(s) of\n"
				"the base game and the DLC for all\n"
				"installed regions of the game.\n\n");
				break;
		case TITLES_NOT_FOUND:
			printf(
				"ERROR: No versions of the game were found."
				"Please make sure you have the latest version(s) of\n"
				"the base game and the DLC\n"
				"for all installed regions of the game.\n\n");
			break;
		case FILE_NOT_FOUND:
			printf(
				"ERROR: File in save data not found. Please make\n"
				"sure that you have played up to the point where\n"
				"you can visit the shop and purchase minigames.\n\n");
			break;
		case (Result)0xC8804478:
		case (Result)0xC8A04555:
		case (Result)0xC8804464:
			printf(
				"ERROR: Save data not found. Please make\n"
				"sure that you have played up to the point where\n"
				"you can visit the shop and purchase minigames.\n\n");
			break;
		default:
			printf(
				"ERROR: Unknown error. Error Code: %08lX\n"
				"Please report this error.\n\n", res);
			break;
	}

	printf("Press START to exit.\n");

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break;
	}

	gfxExit();
	amExit();
	fsExit();
	return 0;
}
