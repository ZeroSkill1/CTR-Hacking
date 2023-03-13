#include <patch.hh>

static const uint32_t crc32_lookup[256] =
{
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

static u32 crc32(u8 *buf, u32 len)
{
    u32 ret = 0xffffffff;

    for(u32 i = 0; i < len; ++i)
        ret = crc32_lookup[(ret & 0xFF) ^ buf[i]] ^ (ret >> 8);

    return ~ret;
}

static std::string stringprintf(const char *f, ...)
{
	va_list va;
	va_start(va, f);

	va_list cpy;
	va_copy(cpy, va);

	int len = vsnprintf(NULL, 0, f, cpy);
	char *charbuf = (char *)malloc(len + 1);

	vsnprintf(charbuf, len + 1, f, va);
	va_end(va);
	va_end(cpy);

	std::string out_str(charbuf);

	free(charbuf);
	return out_str;
}

Result DLC::TitleExists(const DLC::PatchTitle &title)
{
	u64 title_ids[3] =
	{
		0x0004000000000000 | (u64)title.UniqueID, // base title
		0x0004008C00000000 | (u64)title.UniqueID, // dlc title
		0x0004000E00000000 | (u64)title.UniqueID  // update title
	};

	bool uses_dlc = (int)title.Flags & (int)PatchTitleFlags::HasDLC;
	bool uses_update = (int)title.Flags & (int)PatchTitleFlags::HasUpdate;

	AM_TitleEntry title_entries[3];

	u8 count = 1;

	if (uses_dlc)    count++;
	if (uses_update) count++;

	Result res;

	if (R_FAILED(res = AM_GetTitleInfo(MEDIATYPE_SD, count, title_ids, title_entries)))
		return res;

	if ((title_entries[0].version != title.TitleVersion) || (uses_dlc && title_entries[1].version != title.DLCTitleVersion) || (uses_update && title_entries[2].version != title.UpdateTitleVersion))
		return TITLE_VERSION_MISMATCH;

	DLC::SaveData savedata(title.UniqueID, (int)title.Flags & (int)PatchTitleFlags::UsesSaveData ? true : false);

	if (R_FAILED(savedata.CurrentResult) ||
		R_FAILED(savedata.Close(true)))
		return savedata.CurrentResult;

	return 0;
}

Result DLC::TitlesExist(const PatchTitle *titles, u32 *out, int count)
{
	if (!out)
		return INVALID_POINTER;

	Result res = 0;
	u32 ret = 0;

	for (int i = 0; i < count; i++)
	{
		if (R_FAILED(res = DLC::TitleExists(titles[i])) && res != (Result)0xD8A083FA)
			return res;

		if (res == (Result)0xD8A083FA) // am title not found
			continue;

		ret |= (1 << i);
	}

	*out = ret;

	return 0;
}

Result DLC::RustysRealDealBaseball::PatchSingle(const DLC::PatchTitle& title)
{
	Result res;
	u64 size;
	u8 *buf;

	DLC::SaveData savedata(title.UniqueID, true);

	if (R_FAILED(savedata.CurrentResult))
		return savedata.CurrentResult;

	if (R_FAILED(savedata.ReadFileIntoBuffer(buf, "/save00.bin", &size)))
	{
		res = savedata.CurrentResult;
		savedata.Close(true);
		return res;
	}

	if (size != 7076)
	{
		res = savedata.CurrentResult;
		savedata.Close(true);
		return INVALID_SAVE_SIZE;
	}

	// outfits                                ( page, row, column )
	/*
		D60 + 19D4 to 0     = t-shirt              -> 1, 1, 1
		DB4 + 1A74 to 1     = rusty slugger jacket -> 1, 1, 2
		D7C + 1A04 to 2     = business suit        -> 1, 1, 3
	*/

	buf[0xD60] = buf[0xDB4] = buf[0xD7C] = 1;
	buf[0x19D4] = 0; // should be like that by default anyway
	buf[0x1A74] = 1;
	buf[0x1A04] = 2;

	/*
		D84 + 1A14 to 3     = farmer's outfit      -> 1, 2, 1
		D8C + 1A24 to 4     = ninja suit           -> 1, 2, 2
		D94 + 1A34 to 5     = space suit           -> 1, 2, 3
	*/

	buf[0xD84] = buf[0xD8C] = buf[0xD94] = 1;
	buf[0x1A14] = 3;
	buf[0x1A24] = 4;
	buf[0x1A34] = 5;

	/*
		D9C + 1A44 to 6     = basic uniform        -> 1, 3, 1
		DA4 + 1A54 to 7     = pinstripe uniform    -> 1, 3, 2
		DAC + 1A64 to 8     = pro ump outfit       -> 1, 3, 3
	*/

	buf[0xD9C] = buf[0xDA4] = buf[0xDAC] = 1;
	buf[0x1A44] = 6;
	buf[0x1A54] = 7;
	buf[0x1A64] = 8;

	/*
		D6C + 19E4 to 9     = sweat suit           -> 2, 1, 1
		D74 + 19F4 to A     = tropical shirt       -> 2, 1, 2
		D78 + 19FC to B     = school uniform       -> 2, 1, 3
	*/

	buf[0xD6C] = buf[0xD74] = buf[0xD78] = 1;
	buf[0x19E4] = 9;
	buf[0x19F4] = 0xA;
	buf[0x19FC] = 0xB;

	/*
		D80 + 1A0C to C     = western outfit       -> 2, 2, 1
		D88 + 1A1C to D     = kung-fu outfit       -> 2, 2, 2
		D90 + 1A2C to E     = caveman suit         -> 2, 2, 3
	*/

	buf[0xD80] = buf[0xD88] = buf[0xD90] = 1;
	buf[0x1A0C] = 0xC;
	buf[0x1A1C] = 0xD;
	buf[0x1A2C] = 0xE;

	/*
		D98 + 1A3C to F     = manager's jacket     -> 2, 3, 1
		DA0 + 1A4C to 10    = road uniform         -> 2, 3, 2
		DA8 + 1A5C to 11    = short uniform        -> 2, 3, 3
	*/

	buf[0xD98] = buf[0xDA0] = buf[0xDA8] = 1;
	buf[0x1A3C] = 0xF;
	buf[0x1A4C] = 0x10;
	buf[0x1A5C] = 0x11;

	/*
		DB0 + 1A6C to 12    = martian outfit       -> 3, 1, 1
		D68 + 19DC to 13    = workout set          -> 3, 1, 2
		D70 + 19EC to 14    = football uniform     -> 3, 1, 3
	*/

	buf[0xDB0] = buf[0xD68] = buf[0xD70] = 1;
	buf[0x1A6C] = 0x12;
	buf[0x19DC] = 0x13;
	buf[0x19EC] = 0x14;

	// fully advance story.
	buf[0x19D2] = 0x06;

	// // enable dog being your accomplice again.
	// buf[0x45A] = 0xFF;
	// ^ i'm not gonna add this yet; it seems keeping this on will trigger the message again and i'm not sure if that's intended behavior
	// maybe this is random?

	// enable games.
	for (u32 i = 0; i < 10; i++)
		buf[0x3E4 + i] = i;

	// required for enabling games.
	memset(buf + 0x3EE, 0x01, 10);

	// recalculate crc32 checksum.
	*((u32 *)(buf + size - 4)) = crc32(buf, size - 4);

	if (R_FAILED(savedata.WriteFileFromBuffer(buf, "/save00.bin", (u32)size)))
	{
		res = savedata.CurrentResult;
		savedata.Close(true);
		return res;
	}

	if (R_FAILED(savedata.Close()))
	{
		delete [] buf;
		return savedata.CurrentResult;
	}

	delete [] buf;

	return 0;
}

Result DLC::RustysRealDealBaseball::Patch()
{
	Result res;
	u32 titles;
	std::vector<const DLC::PatchTitle *> patch_titles;

	if (R_FAILED(res = DLC::TitlesExist(DLC::RustysRealDealBaseball::Titles, &titles, 3)))
		return res;
	else if (titles == 0)
		return TITLES_NOT_FOUND;

	if (titles & 0x1) // USA
		patch_titles.push_back(&DLC::RustysRealDealBaseball::Titles[0]);

	if (titles & 0x2) // KOR
		patch_titles.push_back(&DLC::RustysRealDealBaseball::Titles[1]);

	if (titles & 0x4) // JPN
		patch_titles.push_back(&DLC::RustysRealDealBaseball::Titles[2]);

	for (const DLC::PatchTitle *&title : patch_titles)
		if (R_FAILED(res = DLC::RustysRealDealBaseball::PatchSingle(*title)))
			return res;

	return 0;
}

static const char GetMcRegionChar(DLC::PatchTitleRegion region)
{
	switch (region)
	{
		case DLC::PatchTitleRegion::EUR:
			return 'P';
		case DLC::PatchTitleRegion::USA:
			return 'E';
		case DLC::PatchTitleRegion::JPN:
			return 'J';
		default:
			return '\0';
	}
}

Result DLC::Minecraft::PatchSingle(const DLC::PatchTitle& title)
{
	u64 save_size;
	Result res;
	u8 *buf;

	DLC::SaveData savedata(title.UniqueID, false, true);

	if (R_FAILED(savedata.CurrentResult))
		return savedata.CurrentResult;

	if (R_FAILED(savedata.ReadFileIntoBuffer(buf, "/options.txt", &save_size)))
	{
		res = savedata.CurrentResult;
		savedata.Close();
		return res;
	}

	/**
	 * The DRM goes as follows: 
	 * 1) Ensure the options.txt file is 128KiB
	 * 2) Ensure the header = the amount of data in the file excluding the header and trailing 0x00 (4 bytes).
	 * 3) Ensure all purchased content is stored in the purchased_items key in this format: "${GAME_PROD_ID}${CONTENT_ID}". All purchased content must be seperated with commas.
	 * 4) The latest update data title must be installed, this contains the actual DLC content.
	 * 5) The DLC must be installed, although it doesn't contain data. The game still checks whether or not it is installed.
	 * 
	 * What we're doing here is just bruteforcing every content ID from 0x00 until 0xFF because the game doesn't check for extra IDs.
	 **/

	bool at_value = false;
	std::string kbuf;
	u32 ns = save_size;

	u8 *out_buf = new u8[131072];

	// Skip the header
	for(u32 i = 4, wl = 4; i < save_size + 4; ++i, ++wl)
	{
		// :
		if (buf[i] == 0x3a)
		{
			at_value = true;

			char region_char = GetMcRegionChar(title.Region);

			if (kbuf == "purchased_items")
			{
				std::string newk = stringprintf("CTRMBD3%c%08lX", region_char, 0x0);

				for (u32 i = 0x1; i < 0xFF; ++i)
				{
					newk += stringprintf(",CTRMBD3%c%08lX", region_char, i);
				}

				// layout in ret is now
				// :<..ids...>\n
				// Layout in chars is now
				// :<...old user ids... (0+)>\n
				// Cut every old user id.
				for (u32 j = 0; i < save_size + 3; ++i, ++j)
				{
					if (buf[i + 1] == '\n')
					{
						ns -= j;
						break;
					}
				}

				out_buf[wl] = ':';
				for(size_t j = 0; j < newk.size(); ++j, ++wl)
					out_buf[wl + 1] = newk[j];
				ns += newk.size();

				*((u32 *)out_buf) = ns;
				continue;
			}

			kbuf.clear();
		}
		else if (buf[i] == '\n')
			at_value = false;
		else if (!at_value)
			kbuf.push_back(buf[i]);

		out_buf[wl] = buf[i];
	}

	delete [] buf;

	if (R_FAILED(savedata.WriteFileFromBuffer(out_buf, "/options.txt", ns)))
	{
		delete [] out_buf;
		res = savedata.CurrentResult;
		savedata.Close();
		return res;
	}

	if (R_FAILED(savedata.Close()))
	{
		delete [] out_buf;
		return savedata.CurrentResult;
	}

	delete [] out_buf;

	return 0;
}

Result DLC::Minecraft::Patch()
{
	Result res;
	u32 titles;
	std::vector<const DLC::PatchTitle *> patch_titles;

	if (R_FAILED(res = DLC::TitlesExist(DLC::Minecraft::Titles, &titles, 3)))
		return res;
	else if (titles == 0)
		return TITLES_NOT_FOUND;

	if (titles & 0x1) // JPN
		patch_titles.push_back(&DLC::Minecraft::Titles[0]);

	if (titles & 0x2) // EUR
		patch_titles.push_back(&DLC::Minecraft::Titles[1]);

	if (titles & 0x4) // USA
		patch_titles.push_back(&DLC::Minecraft::Titles[2]);

	for (const DLC::PatchTitle *&title : patch_titles)
		if (R_FAILED(res = DLC::Minecraft::PatchSingle(*title)))
			return res;

	return 0;
}
