#include "pch.h"
#include "YmTngnModel.h"
#include "YmBase/YmBinaryFormatter.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmTngnModel::YmTngnModel()
{
}

YmTngnModel::~YmTngnModel()
{
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnModel::WriteVector3d(YmBinaryFormatter& output, const YmVector3d& value)
{
	output.WriteDouble(value[0]);
	output.WriteDouble(value[1]);
	output.WriteDouble(value[2]);
}

YmVector3d YmTngnModel::ReadVector3d(YmBinaryFormatter& input)
{
	YmVector3d value;
	value[0] = input.ReadDouble();
	value[1] = input.ReadDouble();
	value[2] = input.ReadDouble();
	return value;
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnModel::SchemaHeader::WriteTo(std::streambuf* pStreamBuf)
{
	YM_IS_TRUE(pStreamBuf != nullptr);
	YmBinaryFormatter out(pStreamBuf);
	out.SetFormatFlags(0);
	out.WriteInt32((int32_t)HeaderCode::FILE_HEADER);
	out.WriteInt32(this->version);
	out.WriteInt32(this->formatterBitFlags);
	out.WriteString(this->fileHeaderText);
}

void YmTngnModel::SchemaHeader::ReadFrom(std::streambuf* pStreamBuf)
{
	YM_IS_TRUE(pStreamBuf != nullptr);
	YmBinaryFormatter input(pStreamBuf);
	input.SetFormatFlags(0);
	YM_IS_TRUE(input.ReadInt32() == (int32_t)HeaderCode::FILE_HEADER);
	this->version = input.ReadInt32();
	this->formatterBitFlags = input.ReadInt32();
	this->fileHeaderText = input.ReadString();
}

////////////////////////////////////////////////////////////////////////////////

constexpr uint8_t currentHeaderSize = 12;

void YmTngnModel::DocHeader::WriteTo(YmBinaryFormatter& formatter)
{
	YmBinaryFormatter::AutoSkipConstSizeBytes<uint8_t>::WriteWrittenSize(formatter, currentHeaderSize);
	formatter.WriteInt64(this->contentPosition);
	formatter.WriteInt32(this->readableVersion);
}

void YmTngnModel::DocHeader::ReadFrom(YmBinaryFormatter& formatter)
{
	YmBinaryFormatter::AutoSkipConstSizeBytes<uint8_t> autoSkip(formatter, currentHeaderSize);
	this->contentPosition = formatter.ReadInt64();
	this->readableVersion = formatter.ReadInt32();
}

////////////////////////////////////////////////////////////////////////////////
