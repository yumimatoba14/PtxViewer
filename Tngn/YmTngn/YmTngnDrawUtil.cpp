#include "pch.h"
#include "YmTngnDrawUtil.h"
#include "YmTngnShaderImpl.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmTngnDrawModelMatrixSetter::YmTngnDrawModelMatrixSetter(YmTngnShaderImpl* pShaderImpl)
	: m_pShader(pShaderImpl)
{
	SetIdentityImpl();
}

YmTngnDrawModelMatrixSetter::~YmTngnDrawModelMatrixSetter()
{
	YM_NOEXCEPT_BEGIN("YmTngnDrawModelMatrixSetter::~YmTngnDrawModelMatrixSetter");
	if (m_pShader) {
		SetIdentity();
		m_pShader = nullptr;
	}
	YM_NOEXCEPT_END;
}

bool YmTngnDrawModelMatrixSetter::Set(const YmTngnModelMatrixPtr& pMatrix)
{
	YM_IS_TRUE(m_pShader);
	if (m_pLastMatrix == pMatrix.get()) {
		return false;
	}

	m_pLastMatrix = pMatrix.get();
	if (m_pLastMatrix) {
		m_pShader->SetModelMatrix(*m_pLastMatrix);
	}
	else {
		SetIdentityImpl();
	}
	return true;
}

bool YmTngnDrawModelMatrixSetter::SetIdentity()
{
	YM_IS_TRUE(m_pShader);
	if (!m_pLastMatrix) {
		return false;
	}

	m_pLastMatrix = nullptr;
	SetIdentityImpl();
	return true;
}

void YmTngnDrawModelMatrixSetter::SetIdentity(YmTngnShaderImpl* pShaderImpl)
{
	YM_ASSERT(pShaderImpl);
	DirectX::XMFLOAT4X4 matrix;
	DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixIdentity());
	pShaderImpl->SetModelMatrix(matrix);
}
