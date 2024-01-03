#pragma once

#include "YmTngnFwd.h"

namespace Ymcpp {

class YmTngnShaderContext
{
public:
	void Init(
		D3DInputLayoutPtr pInputLayout,
		D3DVertexShaderPtr pVS, D3DBufferPtr pVSConstBuffer,
		D3DGeometryShaderPtr pGS, D3DBufferPtr pGSConstBuffer,
		D3DPixelShaderPtr pPS, D3DBufferPtr pPSConstBuffer
	);

	const D3DInputLayoutPtr& GetIAInputLayout() const { return m_pIAInputLayout; }
	const D3DVertexShaderPtr& GetVertexShader() const { return m_pVS; }
	const D3DBufferPtr& GetVSConstantBuffer() const { return m_pVSConstantBuffer; }
	const D3DGeometryShaderPtr& GetGeometryShader() const { return m_pGS; }
	const D3DBufferPtr& GetGSConstantBuffer() const { return m_pGSConstantBuffer; }
	const D3DPixelShaderPtr& GetPixelShader() const { return m_pPS; }
	const D3DBufferPtr& GetPSConstantBuffer() const { return m_pPSConstantBuffer; }

private:
	D3DInputLayoutPtr m_pIAInputLayout;
	D3DVertexShaderPtr m_pVS;
	D3DBufferPtr m_pVSConstantBuffer;
	D3DGeometryShaderPtr m_pGS;
	D3DBufferPtr m_pGSConstantBuffer;
	D3DPixelShaderPtr m_pPS;
	D3DBufferPtr m_pPSConstantBuffer;
};

}
