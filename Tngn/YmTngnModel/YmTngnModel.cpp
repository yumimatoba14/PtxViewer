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

void YmTngnModel::FileHeader::WriteTo(std::streambuf* pStreamBuf)
{
	YM_IS_TRUE(pStreamBuf != nullptr);
	YmBinaryFormatter out(pStreamBuf);
	out.SetFormatFlags(0);
	out.WriteInt32((int32_t)HeaderCode::FILE_HEADER);
	out.WriteInt32(this->version);
	out.WriteInt32(this->formatterBitFlags);
	out.WriteString(this->fileHeaderText);
	out.WriteInt64(this->contentPosition);
}

void YmTngnModel::FileHeader::ReadFrom(std::streambuf* pStreamBuf)
{
	YM_IS_TRUE(pStreamBuf != nullptr);
	YmBinaryFormatter input(pStreamBuf);
	input.SetFormatFlags(0);
	YM_IS_TRUE(input.ReadInt32() == (int32_t)HeaderCode::FILE_HEADER);
	this->version = input.ReadInt32();
	this->formatterBitFlags = input.ReadInt32();
	this->fileHeaderText = input.ReadString();
	this->contentPosition = input.ReadInt64();
}

////////////////////////////////////////////////////////////////////////////////
