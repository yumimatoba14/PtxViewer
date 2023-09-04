#pragma once

#include "YmTngnDmPointBlockList.h"

namespace Ymcpp {

/// <summary>
/// YmTngnDmPointBlockList's implementation to load data from a file.
/// </summary>
class YmTngnDmPointBlockListFile : public YmTngnDmPointBlockList
{
public:
	YmTngnDmPointBlockListFile(std::unique_ptr<YmWin32FileBuf> pBaseFile);
	virtual ~YmTngnDmPointBlockListFile();

private:
	void LoadBlockData(std::unique_ptr<YmWin32FileBuf> pBaseFile);
private:
	std::unique_ptr<YmWin32FileBuf> m_pBaseFile;
	YmMemoryMappedFile m_mmFile;
};

}
