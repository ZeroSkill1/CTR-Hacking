#include <save.hh>

Result DLC::SaveData::OpenExtDataArchive(u32 unique_id)
{
	const u32 path[3] = { MEDIATYPE_SD, unique_id, 0 };
	return FSUSER_OpenArchive(&this->Archive, ARCHIVE_EXTDATA, { PATH_BINARY, 12, path });
}

Result DLC::SaveData::OpenSaveDataArchive(u32 unique_id)
{
	const u32 path[3] = { MEDIATYPE_SD, unique_id, 0x00040000 };
	return FSUSER_OpenArchive(&this->Archive, ARCHIVE_USER_SAVEDATA, { PATH_BINARY, 12, path });
}

DLC::SaveData::SaveData(u32 unique_id, bool is_save, bool is_mc)
{
	this->CurrentResult = (this->IsSave = is_save) ?
		this->OpenSaveDataArchive(unique_id) :
		this->OpenExtDataArchive(unique_id >> 8);
	this->IsClosed = false;
	this->IsMinecraft = is_mc;
}

DLC::SaveData::~SaveData()
{
	if (!this->IsClosed)
	{
		if (R_FAILED(this->Close(true)) && this->CurrentResult == CANNOT_CLOSE_ARCHIVE_FILES_OPEN)
		{
			for (const Handle& handle : this->OpenFiles)
				this->CloseFile(handle);

			this->Close(true);
		}
	}
}

Result DLC::SaveData::OpenFile(Handle *out_handle, const char *path, u32 open_flags, u32 attributes)
{
	if (this->IsClosed)
		return ARCHIVE_CLOSED;
	else if (!out_handle)
		return INVALID_POINTER;

	Handle file;

	char16_t filename[256];

	utf8_to_utf16((u16 *)filename, (const uint8_t *)path, 256);

	if (R_FAILED(this->CurrentResult = FSUSER_OpenFile(&file, this->Archive, fsMakePath(PATH_UTF16, filename), open_flags, attributes)))
		return this->CurrentResult;

	this->OpenFiles.insert(file);
	*out_handle = file;
	return 0;
}

Result DLC::SaveData::ReadFileIntoBuffer(u8 *&out_buf, Handle file, u64 *out_size, bool close_file)
{
	if (this->IsClosed)
		return ARCHIVE_CLOSED;
	else if (!file)
		return INVALID_HANDLE;
	else if (this->OpenFiles.find(file) == this->OpenFiles.end())
		return FILE_NOT_FOUND;
	
	u64 size;
	u8 *buf = nullptr;

	if (!this->IsMinecraft)
	{
		if (R_FAILED(this->CurrentResult = FSFILE_GetSize(file, &size)))
		{
			Result res = this->CurrentResult;
			this->CloseFile(file);
			return res;
		}

		buf = new u8[size];

		u32 cur_read;
		u64 remaining = size;
		u64 offset = 0;
		u32 to_read = remaining > 0x1000 ? 0x1000 : remaining;

		while (offset != size)
		{
			if (R_FAILED(this->CurrentResult = FSFILE_Read(file, &cur_read, offset, buf + (u32)offset, to_read)))
			{
				delete [] buf;
				Result res = this->CurrentResult;
				this->CloseFile(file);
				return cur_read != to_read ? INCOMPLETE_IO : res;
			}

			remaining -= to_read;
			offset += to_read;
			to_read = remaining > 0x1000 ? 0x1000 : remaining;
		}
	}
	else
	{
		// Minecraft: New Nintendo 3DS Edition works a bit differently.
		// You have to read the first u32 in the data first. This is the amount of data *after* the header
		// sizeof(u32) (which is 4) + data size (value of first u32) = max amount of data possible to read.
		u32 total_size = 0;
		u32 read;

		if (R_FAILED(this->CurrentResult = FSFILE_Read(file, &read, 0, (void *)&total_size, sizeof(u32))))
		{
			Result res = this->CurrentResult;
			this->CloseFile(file);
			return read != sizeof(u32) ? INCOMPLETE_IO : res;
		}

		total_size += sizeof(u32);

		// Total size should never be more than 128KiB.
		if (total_size > 131072)
		{
			this->CloseFile(file);
			return INVALID_SAVE_SIZE;
		}

		buf = new u8[total_size];

		if (R_FAILED(this->CurrentResult = FSFILE_Read(file, &read, 0, buf, total_size)))
		{
			Result res = this->CurrentResult;
			this->CloseFile(file);
			delete [] buf;
			return read != total_size ? INCOMPLETE_IO : res;
		}

		size = total_size;
	}

	if (close_file && R_FAILED(this->CloseFile(file)))
	{
		delete [] buf;
		return this->CurrentResult;
	}

	out_buf = buf;
	if (out_size) *out_size = size;
	return 0;
}

Result DLC::SaveData::ReadFileIntoBuffer(u8 *&out_buf, const char *path, u64 *out_size)
{
	Handle file;

	if (R_FAILED(this->CurrentResult = this->OpenFile(&file, path, FS_OPEN_READ)) ||
		R_FAILED(this->CurrentResult = this->ReadFileIntoBuffer(out_buf, file, out_size, true)))
		return this->CurrentResult;

	return 0;
}

Result DLC::SaveData::WriteFileFromBuffer(u8 *buf, Handle file, u32 size, bool close_file)
{
	if (!buf)
		return INVALID_POINTER;
	else if (!file)
		return INVALID_HANDLE;
	else if (this->OpenFiles.find(file) == this->OpenFiles.end())
		return FILE_NOT_FOUND;

	u32 written;

	if ((!this->IsMinecraft && R_FAILED(this->CurrentResult = FSFILE_SetSize(file, size))) ||
		R_FAILED(this->CurrentResult = FSFILE_Write(file, &written, 0LL, buf, size, FS_WRITE_FLUSH)) ||
		(close_file && R_FAILED(this->CurrentResult = this->CloseFile(file))))
		return this->CurrentResult;

	if (written != size)
		return INCOMPLETE_IO;

	return 0;
}

Result DLC::SaveData::WriteFileFromBuffer(u8 *buf, const char *path, u32 size)
{
	Handle file;

	if (R_FAILED(this->CurrentResult = this->OpenFile(&file, path, FS_OPEN_WRITE)) ||
		R_FAILED(this->CurrentResult = this->WriteFileFromBuffer(buf, file, size, true)))
		return this->CurrentResult;

	return 0;
}

Result DLC::SaveData::CloseFile(Handle file)
{
	if (!file)
		return INVALID_HANDLE;
	else if (this->OpenFiles.find(file) == this->OpenFiles.end())
		return FILE_NOT_FOUND;

	if (R_FAILED(this->CurrentResult = FSFILE_Close(file)))
		return this->CurrentResult;

	this->OpenFiles.erase(file);
	return 0;
}

Result DLC::SaveData::Close(bool discard_changes)
{
	if (this->IsClosed)
		return ARCHIVE_CLOSED;
	else if (this->OpenFiles.size() > 0)
		return CANNOT_CLOSE_ARCHIVE_FILES_OPEN;

	if ((this->IsSave && !discard_changes && R_FAILED(this->CurrentResult = FSUSER_ControlArchive(this->Archive, ARCHIVE_ACTION_COMMIT_SAVE_DATA, nullptr, 0, nullptr, 0))) ||
		R_FAILED(this->CurrentResult = FSUSER_CloseArchive(this->Archive)))
		return this->CurrentResult;

	this->IsClosed = true;
	return 0;
}