#pragma once

#include "YmTngnDrawingModel.h"
#include "YmTngnDmTriangleMesh.h"
#include "YmTngnViewConfig.h"
#include <vector>

namespace Ymcpp {

class YmTngnDmObjFiles : public YmTngnDrawingModel
{
public:
	using MeshPtr = std::shared_ptr<YmTngnDmTriangleMesh>;
public:
	YmTngnDmObjFiles(const YmTngnViewConfig& config);
	virtual ~YmTngnDmObjFiles();

public:
	void ReadObjFile(const YmTString& filePath);

protected:
	//virtual bool OnSetPickEnabled(bool bEnable);
	virtual void OnDraw(YmTngnDraw* pDraw);

private:
	YmTngnViewConfig m_config;
	std::vector<MeshPtr> m_meshes;
};

}
