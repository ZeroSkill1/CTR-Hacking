#ifndef __save_hh__
#define __save_hh__

#include <customresult.hh>
#include <unordered_set>
#include <string>
#include <3ds.h>

namespace DLC
{
	class SaveData
	{
	private:
		bool IsSave;
		bool IsClosed;
		bool IsMinecraft;

		FS_Archive Archive;

		std::unordered_set<Handle> OpenFiles;

		Result OpenExtDataArchive(u32 unique_id);
		Result OpenSaveDataArchive(u32 unique_id);

	public:
		SaveData(u32 unique_id, bool is_save = false, bool is_mc = false);
		~SaveData();

		Result CurrentResult;

		Result OpenFile(Handle *out_handle, const char *path, u32 open_flags = FS_OPEN_READ | FS_OPEN_WRITE, u32 attributes = 0);
		Result ReadFileIntoBuffer(u8 *&out_buf, Handle file, u64 *out_size = nullptr, bool close_file = false);
		Result ReadFileIntoBuffer(u8 *&out_buf, const char *path, u64 *out_size = nullptr);
		Result WriteFileFromBuffer(u8 *buf, Handle file, u32 size, bool close_file);
		Result WriteFileFromBuffer(u8 *buf, const char *path, u32 size);
		Result CloseFile(Handle file);
		Result Close(bool discard_changes = false);
	};
}

#endif