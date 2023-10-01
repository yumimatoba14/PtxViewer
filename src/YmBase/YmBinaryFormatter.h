#pragma once

#include "YmBaseFwd.h"
#include <vector>
#include <streambuf>

namespace Ymcpp {

class YmBinaryFormatter
{
public:
	enum FormatFlags {
		BIG_ENDIAN = 0x01,
		ALL_FORMAT_FLAGS = (uint32_t)-1
	};

	// Utility class to skip some bytes.
	// This class is used in case when written/read size are known in advance.
	template<typename SizeType>
	class AutoSkipConstSizeBytes {
	public:
		explicit AutoSkipConstSizeBytes(YmBinaryFormatter& formatter, SizeType nReadByte)
			: m_input(formatter), m_nReadByte(nReadByte)
		{
			ReadSize(&m_nWrittenByte);
			YM_IS_TRUE(m_nReadByte <= m_nWrittenByte);
		}

		~AutoSkipConstSizeBytes()
		{
			if (m_nReadByte < m_nWrittenByte) {
				m_input.ReadBytes(m_nWrittenByte - m_nReadByte);
			}
		}

		static void WriteWrittenSize(YmBinaryFormatter& output, SizeType nByte) { WriteSize(output, nByte); }
	private:
		// nByte is not const reference in order to avoid unexpected type conversion.
		static void WriteSize(YmBinaryFormatter& output, uint8_t& nByte) { output.WriteUInt8(nByte); }

		void ReadSize(uint8_t* pValue) { *pValue = m_input.ReadUInt8(); }
	private:
		YmBinaryFormatter& m_input;
		SizeType m_nReadByte;
		SizeType m_nWrittenByte = 0;
	};
protected:
	YmBinaryFormatter() {}
public:
	explicit YmBinaryFormatter(std::streambuf* pStreamBuf);

	virtual ~YmBinaryFormatter();

	YmBinaryFormatter(const YmBinaryFormatter& other) = default;
	YmBinaryFormatter& operator = (const YmBinaryFormatter& other) = default;

public:
	static YmBinaryFormatter CreateForMemoryImage(std::streambuf* pStreamBuf);

public:
	uint32_t GetFormatFlags() const { return m_formatFlags; }
	void SetFormatFlags(uint32_t value) { m_formatFlags = value; OnFormatFlagsChanged(ALL_FORMAT_FLAGS); }

	bool GetFormatFlag(FormatFlags flag) const { return (m_formatFlags & flag) != 0; }
	void SetFormatFlag(FormatFlags flag, bool enable);

public:
	std::streambuf* GetStreamBuf() { return m_pStreamBuf; }

	int64_t GetCurrentPosition() const;
	void SetCurrentPosition(int64_t pos);

	void Flush();

	void WriteBytes(const char* aByte, size_t nByte);
	void ReadBytes(char* aByte, size_t nByte);
	void ReadBytes(size_t nByte);
public:
	void WriteInt8(int8_t value) { WriteValue<int8_t>(1, value); }
	int8_t ReadInt8() { return ReadValue<int8_t>(1); }

	void WriteUInt8(uint8_t value) { WriteValue<int8_t>(1, (int8_t)value); }
	uint8_t ReadUInt8() { return (uint8_t)ReadValue<int8_t>(1); }

	void WriteInt16(int16_t value) { WriteValue<int16_t>(2, value); }
	int16_t ReadInt16() { return ReadValue<int16_t>(2); }

	void WriteUInt16(uint16_t value) { WriteValue<int16_t>(2, (int16_t)value); }
	uint16_t ReadUInt16() { return (uint16_t)ReadValue<int16_t>(2); }

	void WriteInt32(int32_t value) { WriteValue<int32_t>(4, value); }
	int32_t ReadInt32() { return ReadValue<int32_t>(4); }

	void WriteUInt32(uint32_t value) { WriteValue<int32_t>(4, (int32_t)value); }
	uint32_t ReadUInt32() { return (uint32_t)ReadValue<int32_t>(4); }

	void WriteInt64(int64_t value) { WriteValue<int64_t>(8, value); }
	int64_t ReadInt64() { return ReadValue<int64_t>(8); }

	void WriteUInt64(uint64_t value) { WriteValue<int64_t>(8, (int64_t)value); }
	uint64_t ReadUInt64() { return (uint64_t)ReadValue<int64_t>(8); }

	void WriteFloat(float value) { WriteValue<float>(4, value); }
	float ReadFloat() { return ReadValue<float>(4); }

	void WriteDouble(double value) { WriteValue<double>(8, value); }
	double ReadDouble() { return ReadValue<double>(8); }

	void WriteString(const std::string& value);
	std::string ReadString();

	void WriteFromStreamBuf(std::streambuf* pBuf, int64_t nByte);
	void ReadToStreamBuf(std::streambuf* pBuf, int64_t nByte);
public:
	static bool IsNativeLittleEndianByInt32();
	static bool IsNativeLittleEndianByDouble();

protected:
	void SetStreamBuf(std::streambuf* pStreamBuf) { m_pStreamBuf = pStreamBuf; }

private:
	void OnFormatFlagsChanged(FormatFlags flag);

	template<class T>
	void WriteValue(size_t nByte, std::remove_reference_t<T> value)
	{
		char* aByte = (char*)&value;
		if (m_isReverseEndian) {
			std::reverse(aByte, aByte + sizeof(T));
		}
		WriteBytes(aByte, nByte);
	}

	template<class T>
	T ReadValue(size_t nByte)
	{
		T value = 0;
		YM_ASSERT(nByte <= sizeof(T));
		char* aByte = (char*)&value;
		ReadBytes(aByte, nByte);
		if (m_isReverseEndian) {
			std::reverse(aByte, aByte + sizeof(T));
		}
		return value;
	}

private:
	uint32_t m_formatFlags = 0;
	std::streambuf* m_pStreamBuf = nullptr;
	bool m_isReverseEndian = false;
};

}
