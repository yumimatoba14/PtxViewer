#pragma once

#include "YmTngnDrawingModel.h"
#include "YmTngnDmTriangleMesh.h"
#include "YmTngnViewConfig.h"
#include <vector>

namespace Ymcpp {

class YmTngnDmObjFiles : public YmTngnDrawingModel
{
public:
	using MeshPtr = YmTngnDmTriangleMeshPtr;
public:
	YmTngnDmObjFiles(const YmTngnViewConfig& config);
	virtual ~YmTngnDmObjFiles();

public:
	void ReadObjFile(const YmTString& filePath);
	std::vector<YmTngnDmTriangleMeshPtr> FindPickedMesh(YmTngnPickTargetId id) const;
protected:
	virtual bool OnSetPickEnabled(bool isEnable);
	virtual void OnDraw(YmTngnDraw* pDraw);

private:
	YmTngnViewConfig m_config;
	std::vector<MeshPtr> m_meshes;
};

}
