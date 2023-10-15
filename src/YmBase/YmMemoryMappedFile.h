#pragma once

#include "YmWin32.h"
#include "YmBase.h"

namespace Ymcpp {

class YmMemoryMappedFile
{
public:
	/// <summary>
	/// RAII class to have a meory allocated by MapViewOfFile().
	/// </summary>
	class MappedPtr {
	public:
		MappedPtr() noexcept : MappedPtr(nullptr, 0) {}
		MappedPtr(void* pViewBegin, DWORD dataOffset) noexcept
			: m_pData(pViewBegin), m_dataOffsetByte(dataOffset)
		{}
		MappedPtr(const MappedPtr&) = delete;
		MappedPtr(MappedPtr&& other) noexcept : MappedPtr() { MoveFrom(other); }
		/*virtual*/~MappedPtr() { UnmapNoexcept(); }

		MappedPtr& operator = (const MappedPtr&) = delete;
		MappedPtr& operator = (MappedPtr&& other) noexcept
		{
			if (this != &other) {
				UnmapNoexcept();
				MoveFrom(other);
			}
			return *this;
		}
		MappedPtr& operator = (nullptr_t)
		{
			Unmap();
			return *this;
		}

		void* Get() const { return IsNull() ? nullptr : (static_cast<char*>(m_pData) + m_dataOffsetByte); }

		template<class T>
		T* ToArray() const { return (T*)Get(); }

		template<class T>
		const T* ToConstArray() const { return (T*)Get(); }

		operator bool() { return !IsNull(); }
		bool IsNull() const { return m_pData == nullptr; }

	private:
		void UnmapNoexcept() noexcept
		{
			if (*this) {
				UnmapNoexceptImpl();
			}
		}
		void UnmapNoexceptImpl() noexcept;
		void Unmap();
		void MoveFrom(MappedPtr& other) noexcept
		{
			MoveToLeft(m_pData, other.m_pData, nullptr);
			MoveToLeft(m_dataOffsetByte, other.m_dataOffsetByte, 0);
		}

	private:
		void* m_pData = nullptr;
		DWORD m_dataOffsetByte;
	};

	enum Mode {
		MODE_READ,
		MODE_READ_WRITE,
		MODE_NEW_FILE,
		MODE_TEMP_FILE
	};
public:
	YmMemoryMappedFile() noexcept {}
	/*virtual*/ ~YmMemoryMappedFile() { CloseNoexcept(); }

	YmMemoryMappedFile(const YmMemoryMappedFile&) = delete;
	YmMemoryMappedFile(YmMemoryMappedFile&& other) noexcept
		: YmMemoryMappedFile()
	{
		MoveFrom(other);
	}

	YmMemoryMappedFile& operator = (const YmMemoryMappedFile&) = delete;
	YmMemoryMappedFile& operator = (YmMemoryMappedFile&& other) noexcept
	{
		if (this != &other) {
			CloseNoexcept();
			MoveFrom(other);
		}
		return *this;
	}

	bool OpenToRead(LPCTSTR pFilePath) { return OpenImpl(pFilePath, MODE_READ, 0); }
	bool OpenNewFile(LPCTSTR pFilePath, uint64_t newFileSize)
	{
		return OpenImpl(pFilePath, MODE_NEW_FILE, newFileSize);
	}
	bool OpenTempFile(LPCTSTR pFilePath, uint64_t newFileSize)
	{
		return OpenImpl(pFilePath, MODE_TEMP_FILE, newFileSize);
	}
	void AttachFileToRead(HANDLE hFile)
	{
		AttachFileImpl(hFile, MODE_READ, 0);
	}
	/// <summary>
	/// 
	/// </summary>
	/// <param name="hFile"></param>
	/// <param name="fileSize">It can be 0 when this object modifies the current size of hFile.</param>
	void AttachFileToWrite(HANDLE hFile, uint64_t fileSize)
	{
		AttachFileImpl(hFile, MODE_READ_WRITE, fileSize);
	}

	void Close();

	bool IsOpened() const { return m_hFileMapping != nullptr; }
	MappedPtr MapView(uint64_t filePos, size_t viewSize);

	DWORD PrepareAllocationGranularity();

private:
	bool OpenImpl(LPCTSTR pFilePath, Mode mode, uint64_t fileSize);
	void AttachFileImpl(HANDLE hFile, Mode mode, uint64_t fileSize);
	bool CreateFileMapppingObject(HANDLE hFile, Mode mode, uint64_t fileSize);

	void CloseNoexcept() noexcept
	{
		if (IsOpened()) {
			CloseNoexceptImpl();
		}
	}
	void CloseNoexceptImpl() noexcept;

	void MoveFrom(YmMemoryMappedFile& other) noexcept
	{
		MoveToLeft(m_hFileToClose, other.m_hFileToClose, INVALID_HANDLE_VALUE);
		MoveToLeft(m_hFileMapping, other.m_hFileMapping, nullptr);
		MoveToLeft(m_allocationGranularity, other.m_allocationGranularity, 0);
		MoveToLeft(m_isWriteMode, other.m_isWriteMode, false);
	}

private:
	HANDLE m_hFileToClose = INVALID_HANDLE_VALUE;
	HANDLE m_hFileMapping = nullptr;
	DWORD m_allocationGranularity = 0;
	bool m_isWriteMode = false;
};

}
