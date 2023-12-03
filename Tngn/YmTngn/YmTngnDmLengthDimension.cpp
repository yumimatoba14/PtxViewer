#include "pch.h"
#include "YmTngnDmLengthDimension.h"
#include "YmTngnDraw.h"
#include <sstream>

using namespace std;
using namespace Ymcpp;
using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////

YmTngnDmLengthDimension::YmTngnDmLengthDimension()
{
	SetColor(YmRgba4b(255, 0, 0));
}
YmTngnDmLengthDimension::YmTngnDmLengthDimension(const YmVector3d& point0, const YmVector3d& point1)
	: YmTngnDmLengthDimension()
{
	SetPointAt(0, point0);
	SetPointAt(1, point1);
	SetLengthText((point0 - point1).GetLength());
}

YmTngnDmLengthDimension::~YmTngnDmLengthDimension()
{
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmLengthDimension::SetColor(const YmRgba4b& color)
{
	m_color = color;
	m_aTerminalPoint[0].rgba = m_color.ToUint32();
	m_aTerminalPoint[1].rgba = m_color.ToUint32();
}

void YmTngnDmLengthDimension::SetPointAt(int i, const YmVector3d& coord)
{
	YM_IS_TRUE(0 <= i && i < 2);
	m_aTerminalPoint[i].position = YmVectorUtil::StaticCast<XMFLOAT3>(coord);
}

void YmTngnDmLengthDimension::SetLengthText(double length)
{
	ostringstream oss;
	oss << length;
	m_text = oss.str();
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmLengthDimension::OnDraw(YmTngnDraw* pDraw)
{
	pDraw->DrawLineList(m_aTerminalPoint, 2);
	if (!m_text.empty() && !pDraw->IsProgressiveViewFollowingFrame()) {
		YmVector3d origin = 0.5 * (GetPointAt(0) + GetPointAt(1));
		pDraw->Register3DText(origin, m_text);
	}
}

////////////////////////////////////////////////////////////////////////////////
