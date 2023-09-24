#include "pch.h"
#include "YmTngnDrawingModel.h"
#include "YmTngnDraw.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmTngnDrawingModel::YmTngnDrawingModel()
{
}

YmTngnDrawingModel::~YmTngnDrawingModel()
{
}

////////////////////////////////////////////////////////////////////////////////

bool YmTngnDrawingModel::SetPickEnabled(bool isEnable)
{
	m_isPickEnabled = OnSetPickEnabled(isEnable);
	return m_isPickEnabled;
}

void YmTngnDrawingModel::Draw(YmTngnDraw* pDraw)
{
	OnDraw(pDraw);
}

std::vector<YmTngnPointListVertex> YmTngnDrawingModel::FindPickedPoints(YmTngnPickTargetId id)
{
	return OnFindPickedPoints(id);
}

////////////////////////////////////////////////////////////////////////////////

bool YmTngnDrawingModel::OnSetPickEnabled(bool bEnable)
{
	return false;
}

std::vector<YmTngnPointListVertex> YmTngnDrawingModel::OnFindPickedPoints(YmTngnPickTargetId id)
{
	return vector<YmTngnPointListVertex>();
}

////////////////////////////////////////////////////////////////////////////////
