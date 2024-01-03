#include "pch.h"
#include "YmTngnShaderContext.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

void YmTngnShaderContext::Init(
	D3DInputLayoutPtr pInputLayout,
	D3DVertexShaderPtr pVS, D3DBufferPtr pVSConstBuffer,
	D3DGeometryShaderPtr pGS, D3DBufferPtr pGSConstBuffer,
	D3DPixelShaderPtr pPS, D3DBufferPtr pPSConstBuffer
)
{
	m_pIAInputLayout = pInputLayout;
	m_pVS = move(pVS);
	m_pVSConstantBuffer = move(pVSConstBuffer);
	m_pGS = move(pGS);
	m_pGSConstantBuffer = move(pGSConstBuffer);
	m_pPS = move(pPS);
	m_pPSConstantBuffer = move(pPSConstBuffer);
}

////////////////////////////////////////////////////////////////////////////////
