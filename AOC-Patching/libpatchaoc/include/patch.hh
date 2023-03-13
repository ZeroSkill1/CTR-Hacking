#ifndef __patch_hh__
#define __patch_hh__

#include <customresult.hh>
#include <cstdarg>
#include <cstring>
#include <stdio.h>
#include <save.hh>
#include <vector>

namespace DLC
{
	// expand this enum when adding new games
	enum class PatchType
	{
		Minecraft              = 1,
		RustysRealDealBaseball = 2,
	};

	enum class PatchTitleRegion
	{
		EUR = 1,
		USA = 2,
		JPN = 3,
		KOR = 4,
		TWN = 5,
	};

	enum class PatchTitleFlags
	{
		HasDLC       = 0x1,
		HasUpdate    = 0x2,
		IsKTROnly    = 0x4,
		UsesExtData  = 0x8,
		UsesSaveData = 0x10,
	};

	typedef struct PatchTitle
	{
		u32 UniqueID;
		PatchTitleFlags Flags;
		u16 TitleVersion;
		u16 DLCTitleVersion;
		u16 UpdateTitleVersion;
		PatchTitleRegion Region;
	} PatchTitle;

	namespace Minecraft
	{
		static const PatchTitle Titles[3] = 
		{
			{ 0x17FD00, (PatchTitleFlags)( (int)PatchTitleFlags::HasUpdate | (int)PatchTitleFlags::HasDLC | (int)PatchTitleFlags::IsKTROnly | (int)PatchTitleFlags::UsesExtData ), 48, 48, 9424, PatchTitleRegion::JPN },
			{ 0x17CA00, (PatchTitleFlags)( (int)PatchTitleFlags::HasUpdate | (int)PatchTitleFlags::HasDLC | (int)PatchTitleFlags::IsKTROnly | (int)PatchTitleFlags::UsesExtData ), 16, 48, 9392, PatchTitleRegion::EUR },
			{ 0x1B8700, (PatchTitleFlags)( (int)PatchTitleFlags::HasUpdate | (int)PatchTitleFlags::HasDLC | (int)PatchTitleFlags::IsKTROnly | (int)PatchTitleFlags::UsesExtData ), 16, 48, 9408, PatchTitleRegion::USA },
		};

		Result PatchSingle(const DLC::PatchTitle& title);
		Result Patch();
	}

	namespace RustysRealDealBaseball
	{
		static const PatchTitle Titles[3]
		{
			{ 0x106400, (PatchTitleFlags)( (int)PatchTitleFlags::HasDLC | (int)PatchTitleFlags::UsesSaveData ),   16, 32, 0xFFFF, PatchTitleRegion::USA },
			{ 0x126200, (PatchTitleFlags)( (int)PatchTitleFlags::HasDLC | (int)PatchTitleFlags::UsesSaveData ),    0,  0, 0xFFFF, PatchTitleRegion::KOR },
			{ 0x0D2900, (PatchTitleFlags)( (int)PatchTitleFlags::HasDLC | (int)PatchTitleFlags::UsesSaveData ), 1056,  0, 0xFFFF, PatchTitleRegion::JPN },
		};

		Result PatchSingle(const DLC::PatchTitle& title);
		Result Patch();
	}

	Result TitleExists(const PatchTitle &title);
	Result TitlesExist(const PatchTitle *titles, u32 *out, int count);
	
	inline Result Patch(PatchType type)
	{
		switch (type)
		{
			case PatchType::Minecraft:
				return DLC::Minecraft::Patch();
			case PatchType::RustysRealDealBaseball:
				return DLC::RustysRealDealBaseball::Patch();
			default:
				return INVALID_PATCH_TYPE;
		}
	}
}

#endif