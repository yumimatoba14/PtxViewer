#include "pch.h"
#include "YmMemoryMappedFile.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

#if 0
YmMemoryMappedFile::YmMemoryMappedFile()
{
}

YmMemoryMappedFile::~YmMemoryMappedFile()
{
}
#endif

////////////////////////////////////////////////////////////////////////////////

bool YmMemoryMappedFile::OpenImpl(LPCTSTR pFilePath, Mode mode, uint64_t fileSize)
{
	YM_IS_TRUE(pFilePath);

	DWORD desiredAccess = GENERIC_READ;
	DWORD shareMode = FILE_SHARE_READ;
	DWORD creationDisposition = OPEN_EXISTING;
	DWORD fileAttrs = FILE_ATTRIBUTE_NORMAL;
	switch (mode) {
	case MODE_READ:
		break;
	case MODE_NEW_FILE:
	case MODE_TEMP_FILE:
		desiredAccess = GENERIC_READ | GENERIC_WRITE;
		creationDisposition = CREATE_ALWAYS;
		if (mode == MODE_TEMP_FILE) {
			fileAttrs = FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE;
		}
		break;
	case MODE_READ_WRITE:
		YM_THROW_ERROR("Not supported value.");
	default:
		YM_ASSERT(false);	// TODO: add correct cases.
	}

	Close();

	m_hFileToClose = CreateFile(
		pFilePath, desiredAccess, shareMode, NULL,
		creationDisposition, fileAttrs, NULL
	);
	if (m_hFileToClose == INVALID_HANDLE_VALUE) {
		YM_WRITE_LOG("CreateFile");
		return false;
	}

	return CreateFileMapppingObject(m_hFileToClose, mode, fileSize);
}

/// <summary>
/// 
/// </summary>
/// <param name="hFile">File to be mapped. This object doesn't close this file handle.</param>
/// <param name="mode"></param>
/// <param name="fileSize"></param>
void YmMemoryMappedFile::AttachFileImpl(HANDLE hFile, Mode mode, uint64_t fileSize)
{
	YM_IS_TRUE(hFile != INVALID_HANDLE_VALUE && hFile != nullptr);;
	Close();

	bool isOk = CreateFileMapppingObject(hFile, mode, fileSize);
	if (!isOk) {
		YM_THROW_ERROR("CreateFileMapppingObject");
	}
}

bool YmMemoryMappedFile::CreateFileMapppingObject(HANDLE hFile, Mode mode, uint64_t fileSize)
{
	DWORD pageProtect = PAGE_READONLY;
	m_isWriteMode = false;
	switch (mode) {
	case MODE_READ:
		break;
	case MODE_NEW_FILE:
	case MODE_TEMP_FILE:
		YM_IS_TRUE(0 < static_cast<int64_t>(fileSize));
		//break; // fall through
	case MODE_READ_WRITE:
		pageProtect = PAGE_READWRITE;
		m_isWriteMode = true;
		break;
	default:
		YM_ASSERT(false);	// TODO: add correct cases.
	}

	m_hFileMapping = ::CreateFileMapping(
		hFile, nullptr, pageProtect, DWORD(fileSize >> 32), (DWORD)fileSize, nullptr
	);
	if (m_hFileMapping == nullptr) {
		YM_WRITE_LOG("CreateFileMapping");
		return false;
	}

	return true;
}

void YmMemoryMappedFile::Close()
{
	if (m_hFileMapping != nullptr) {
		::CloseHandle(m_hFileMapping);
		m_hFileMapping = nullptr;
	}
	if (m_hFileToClose != INVALID_HANDLE_VALUE) {
		::CloseHandle(m_hFileToClose);
		m_hFileToClose = INVALID_HANDLE_VALUE;
	}
}

YmMemoryMappedFile::MappedPtr YmMemoryMappedFile::MapView(uint64_t filePos, size_t nDataByte)
{
	DWORD granularity = PrepareAllocationGranularity();
	uint64_t viewBegin = (filePos / granularity) * granularity;
	DWORD offsetByte = static_cast<DWORD>(filePos - viewBegin);
	size_t viewSize = offsetByte + nDataByte;
	DWORD desiredAccess = FILE_MAP_READ;
	if (m_isWriteMode) {
		desiredAccess = FILE_MAP_WRITE;
	}
	MappedPtr pMemory(::MapViewOfFile(
		m_hFileMapping, desiredAccess, DWORD(viewBegin >> 32), DWORD(viewBegin), viewSize
	), offsetByte);
	return pMemory;
}

DWORD YmMemoryMappedFile::PrepareAllocationGranularity()
{
	if (m_allocationGranularity == 0) {
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		m_allocationGranularity = sysInfo.dwAllocationGranularity;
		YM_IS_TRUE(0 < m_allocationGranularity);
	}
	return m_allocationGranularity;
}

////////////////////////////////////////////////////////////////////////////////

void YmMemoryMappedFile::CloseNoexceptImpl() noexcept
{
	YM_NOEXCEPT_BEGIN("YmMemoryMappedFile::CloseNoexceptImpl");
	Close();
	YM_NOEXCEPT_END;
}

////////////////////////////////////////////////////////////////////////////////

void YmMemoryMappedFile::MappedPtr::UnmapNoexceptImpl() noexcept
{
	YM_NOEXCEPT_BEGIN("YmMemoryMappedFile::MappedPtr::UnmapNoexcept");
	Unmap();
	YM_NOEXCEPT_END;
}

void YmMemoryMappedFile::MappedPtr::Unmap()
{
	if (m_pData) {
		BOOL isOk = ::UnmapViewOfFile(m_pData);
		if (!isOk) {
			YM_THROW_ERROR("UnmapViewOfFile");
		}
		m_pData = nullptr;
		m_dataOffsetByte = 0;
	}
}
