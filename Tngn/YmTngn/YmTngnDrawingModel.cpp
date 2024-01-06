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

////////////////////////////////////////////////////////////////////////////////

bool YmTngnDrawingModel::OnSetPickEnabled(bool bEnable)
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////
