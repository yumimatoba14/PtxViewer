#pragma once

#include "YmTngnFwd.h"
#include "YmTngnVectorUtil.h"

namespace Ymcpp {

struct YmTngnPointListVertex
{
	DirectX::XMFLOAT3 position;
	UINT rgba;
};

struct YmTngnPickedPoint {
	YmTngnPickTargetId id;
	YmVector3d positionInModel;
	UINT rgba;
};

struct YmTngnTriangleVertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normalDir;
	UINT rgba;
};

}
