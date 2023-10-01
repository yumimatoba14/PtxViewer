#include "pch.h"
#include "YmTngnDmPointBlockListFile.h"
#include <sstream>

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmTngnDmPointBlockListFile::YmTngnDmPointBlockListFile(std::unique_ptr<YmWin32FileBuf> pBaseFile)
{
	LoadBlockData(move(pBaseFile));
}

YmTngnDmPointBlockListFile::~YmTngnDmPointBlockListFile()
{
}

////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Read a file image written by YmTngnPointBlockListBuilder.
/// </summary>
/// <param name="pBaseFile"></param>
void YmTngnDmPointBlockListFile::LoadBlockData(std::unique_ptr<YmWin32FileBuf> pBaseFile)
{
	YM_IS_TRUE(pBaseFile);
	m_pBaseFile = move(pBaseFile);
	m_mmFile.AttachFileToRead(m_pBaseFile->GetHandle());

	YM_IS_TRUE(m_mmFile.IsOpened());
	m_pBaseFile->pubseekpos(0);
	ClearInstance();

	YmTngnModel::SchemaHeader schemaHeader;
	schemaHeader.ReadFrom(m_pBaseFile.get());
	if (schemaHeader.fileHeaderText != YmTngnModel::HEAD_TEXT_POINT_BLOCK_LIST) {
		YM_THROW_ERROR("This file is not point-block-list file.");
	}

	YmBinaryFormatter input(m_pBaseFile.get());
	input.SetFormatFlags(schemaHeader.formatterBitFlags);

	YmTngnModel::DocHeader docHeader;
	docHeader.ReadFrom(input);
	if (YmTngnModel::CURRENT_FILE_VERSION < docHeader.readableVersion) {
		ostringstream oss;
		oss << "This module cannot read this file.(readableVersion = " << docHeader.readableVersion << ")";
		YM_THROW_ERROR(oss.str().c_str());
	}

	input.SetCurrentPosition(docHeader.contentPosition);
	size_t nBlock = static_cast<size_t>(input.ReadInt32());
	ReserveInstanceList(nBlock);
	for (size_t iBlock = 0; iBlock < nBlock; ++iBlock) {
		YmTngnModel::PointBlockHeader blockHeader;
		input.ReadBytes(reinterpret_cast<char*>(&blockHeader), sizeof(YmTngnModel::PointBlockHeader));

		InstanceData instance;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				instance.localToGlobalMatrix.m[i][j] =
					static_cast<float>(blockHeader.localToGlobalMatrix[i * 4 + j]);
			}
		}

		instance.aabb.Extend(blockHeader.aAabbPoint[0]);
		instance.aabb.Extend(blockHeader.aAabbPoint[1]);
		instance.pPointBlock = make_shared<YmTngnDmExclusiveLodPointList>(m_mmFile, blockHeader.firstBytePos);
		AddInstance(move(instance));
	}

	int8_t hasScannerPos = input.ReadInt8();
	YmVector3d scannerPos = YmTngnModel::ReadVector3d(input);
	if (hasScannerPos) {
		SetScannerPosition(scannerPos);
	}
}


////////////////////////////////////////////////////////////////////////////////
