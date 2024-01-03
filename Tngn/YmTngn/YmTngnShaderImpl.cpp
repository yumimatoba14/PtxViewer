#include "pch.h"
#include "YmTngnShaderImpl.h"
#include <d3dcompiler.h>
#include <atlbase.h>
#include <atlconv.h>
#include "YmTngnVectorUtil.h"
#include "YmViewOp.h"
#include "YmTngnViewConfig.h"
#include "YmTngnDmDrawableObjectList.h"
#include "YmBase/YmFilePath.h"
#include "YmBase/YmDebugOutputStream.h"

using namespace std;
using namespace Ymcpp;
using namespace DirectX;

#define VS_COMPILE_TARGET "vs_5_0"

////////////////////////////////////////////////////////////////////////////////

YmTngnShaderImpl::YmTngnShaderImpl(const YmTngnViewConfig& config, const D3DDevicePtr& pDevice, const D3DDeviceContextPtr& pDc)
	: m_pDevice(pDevice), m_pDc(pDc), m_scannerPosition(YmVector3d::MakeZero())
{
	m_pointSize = config.GetDoubleValue(YmTngnViewConfig::POINT_SIZE);
	m_fovAngleYDeg = config.GetDoubleValue(YmTngnViewConfig::FOV_ANGLE_Y_DEG);
	m_viewNearZ = config.GetDoubleValue(YmTngnViewConfig::PERSPECTIVE_VIEW_NEAR_Z);
	m_viewFarZ = config.GetDoubleValue(YmTngnViewConfig::PERSPECTIVE_VIEW_FAR_Z);
	m_scannerDistanceUpperBound = config.GetDoubleValue(YmTngnViewConfig::SCANNER_DISTANCE_UB);
	m_scannerDistanceDepthOffset = config.GetDoubleValue(YmTngnViewConfig::SCANNER_DISTANCE_DEPTH_OFFSET);
	XMStoreFloat4x4(&m_modelMatrix, XMMatrixIdentity());
	Initialize();
}

YmTngnShaderImpl::~YmTngnShaderImpl()
{
}

////////////////////////////////////////////////////////////////////////////////

YmViewOp& YmTngnShaderImpl::GetViewOp()
{
	return *m_pViewOp;
}

void YmTngnShaderImpl::SetProgressiveViewMode(bool enableProgressiveView, bool isFollowingFrame)
{
	m_isProgressiveViewMode = enableProgressiveView;
	m_isProgressiveViewFollowingFrame = enableProgressiveView && isFollowingFrame;
}

XMMATRIX YmTngnShaderImpl::GetModelToViewMatrix() const
{
	return XMMatrixMultiply(XMLoadFloat4x4(&m_modelMatrix), m_pViewOp->GetViewMatrix());
}

XMMATRIX YmTngnShaderImpl::GetModelToProjectionMatrix() const
{
	return XMMatrixMultiply(GetModelToViewMatrix(), this->GetProjectionMatrix(GetAspectRatio()));
}

YmDx11MappedSubResource YmTngnShaderImpl::MapDynamicBuffer(const D3DBufferPtr& pDynamicBuffer)
{
	return MapResource(pDynamicBuffer, D3D11_MAP_WRITE_DISCARD);
}

YmDx11MappedSubResource YmTngnShaderImpl::MapResource(const D3DResourcePtr& pResource, D3D11_MAP mapType)
{
	YM_IS_TRUE(pResource != nullptr);
	UINT subResourceId = 0;
	UINT mapFlags = 0;
	D3D11_MAPPED_SUBRESOURCE mappedData;
	ZeroMemory(&mappedData, sizeof(D3D11_MAPPED_SUBRESOURCE));

	HRESULT hr = m_pDc->Map(pResource.Get(), subResourceId, mapType, mapFlags, &mappedData);
	if (FAILED(hr)) {
		YM_THROW_ERROR("Map");
	}

	return YmDx11MappedSubResource(m_pDc, pResource, move(mappedData));
}

void YmTngnShaderImpl::ClearTransparentObject()
{
	if (m_pTransparentObjectList) {
		m_pTransparentObjectList->ClearObject();
	}
}

void YmTngnShaderImpl::RegisterTransparentObject(
	const YmTngnModelMatrixPtr& pMatrix, const YmTngnDrawableObjectPtr& pObject
)
{
	if (!m_pTransparentObjectList) {
		m_pTransparentObjectList = make_shared<YmTngnDmDrawableObjectList>();
	}
	if (pObject) {
		m_pTransparentObjectList->AddObject(pMatrix, pObject);
	}
}

void YmTngnShaderImpl::Clear2DText()
{
	m_2dTextList.clear();
}

void YmTngnShaderImpl::Register2DText(LPCWSTR pText, const YmVector2i& origin)
{
	if (pText == nullptr) {
		return;
	}
	YmTngn2DText textData;
	textData.SetText(pText);
	textData.SetOrigin(origin);
	m_2dTextList.push_back(move(textData));
}

void YmTngnShaderImpl::Register3DText(const YmVector3d& origin, const std::string& text)
{
	YmVector2i originViewCoord;
	bool isOk = ConvertGlobalCoordToViewIntCoord(origin, &originViewCoord);
	if (isOk) {
		Register2DText(LPCWSTR(CA2W(text.c_str())), originViewCoord);
	}
}

void YmTngnShaderImpl::DrawPointList(
	const D3DBufferPtr& pVertexBuf, size_t vertexSize, size_t nVertex
)
{
	YM_ASSERT(nVertex <= UINT_MAX);
	m_pDc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	ID3D11Buffer* apVB[1] = { pVertexBuf.Get() };
	UINT aVertexSize[1] = { (UINT)vertexSize };
	UINT aOffset[1] = { 0 };
	m_pDc->IASetVertexBuffers(0, 1, apVB, aVertexSize, aOffset);

	SetShaderContext(m_pointListSc);
	m_pDc->Draw((UINT)nVertex, 0);
}

void YmTngnShaderImpl::DrawPickablePointList(
	const D3DBufferPtr& pVertexBuf, const D3DBufferPtr& pPickIdBuf, size_t nVertex
)
{
	YM_ASSERT(nVertex <= UINT_MAX);
	m_pDc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	ID3D11Buffer* apVB[2] = { pVertexBuf.Get(), pPickIdBuf.Get() };
	UINT aVertexSize[2] = { (UINT)sizeof(YmTngnPointListVertex), (UINT)sizeof(YmTngnPickTargetId) };
	UINT aOffset[2] = { 0, 0 };
	m_pDc->IASetVertexBuffers(0, 2, apVB, aVertexSize, aOffset);

	SetShaderContext(m_pickablePointListSc);
	m_pDc->Draw((UINT)nVertex, 0);
}

void YmTngnShaderImpl::DrawTriangleList(
	const D3DBufferPtr& pVertexBuf, const D3DBufferPtr& pIndexBuf, size_t nIndex
)
{
	YM_ASSERT(nIndex <= UINT_MAX);
	m_pDc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ID3D11Buffer* apVB[1] = { pVertexBuf.Get() };
	UINT aVertexSize[1] = { (UINT)sizeof(YmTngnTriangleVertex) };
	UINT aOffset[1] = { 0 };
	m_pDc->IASetVertexBuffers(0, 1, apVB, aVertexSize, aOffset);
	m_pDc->IASetIndexBuffer(pIndexBuf.Get(), DXGI_FORMAT_R32_UINT, 0);

	SetShaderContext(m_triangleListSc);
	m_pDc->DrawIndexed((UINT)nIndex, 0, 0);
}

void YmTngnShaderImpl::DrawLineList(
	const D3DBufferPtr& pVertexBuf, size_t nVertex
)
{
	YM_ASSERT(nVertex <= UINT_MAX);
	m_pDc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	//m_pDc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	ID3D11Buffer* apVB[1] = { pVertexBuf.Get() };
	UINT aVertexSize[1] = { (UINT)sizeof(YmTngnPointListVertex) };
	UINT aOffset[1] = { 0 };
	m_pDc->IASetVertexBuffers(0, 1, apVB, aVertexSize, aOffset);

	SetShaderContext(m_lineListSc);
	m_pDc->Draw((UINT)nVertex, 0);
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnShaderImpl::Initialize()
{
	m_pViewOp = make_unique<YmViewOp>();
	m_pViewOp->AddOnChangedCallback([this]() {
		m_isNeedUpdateShaderParam = true;
	});
	m_pShaderParamConstBuf = CreateConstantBuffer(sizeof(ShaderParam));
	InitializeShaderContextsForNormalRendering();
	InitializeShaderContextsForPickableRendering();
	InitializeShaderContextsForTriangleListNormalRendering();
	InitializeShaderContextsForLineListNormalRendering();
}

void YmTngnShaderImpl::UpdateShaderParam()
{
	YM_IS_TRUE(m_pShaderParamConstBuf);

	ShaderParam shaderParam;
	XMStoreFloat4x4(&(shaderParam.viewMatrix), XMMatrixTranspose(GetModelToViewMatrix()));
	XMStoreFloat4x4(&(shaderParam.projectionMatrix), XMMatrixIdentity());
	double aspectRatio = GetAspectRatio();
	XMStoreFloat4x4(&(shaderParam.projectionMatrix), XMMatrixTranspose(
		GetProjectionMatrix(aspectRatio)
	));
	shaderParam.pixelSizeX = static_cast<float>(0 < m_viewSize[0] ? 2.0 / m_viewSize[0] : 1);
	shaderParam.pixelSizeY = static_cast<float>(0 < m_viewSize[1] ? 2.0 / m_viewSize[1] : 1);

	// See hlsl file for details of shaderParam's pointSizeX and pointSizeY.
	if (m_pointSize < 0) {
		// Set in pixel. (-m_pointSize is size in pixel.)
		shaderParam.pointSizeX = float(m_pointSize * shaderParam.pixelSizeX);
		shaderParam.pointSizeY = float(m_pointSize * shaderParam.pixelSizeY);
	}
	else {
		// Set length in model space. (m_pointSize is size in model space.)
		// Drawn range in Y direction is defined by FovAngleY.
		// Drawn range in X directino is defined by FovAgnleY and aspectRatio.
		// So size in X direction should be adjusted with aspectRatio.
		const double tanY = tan(m_fovAngleYDeg * 0.5 * M_PI / 180);
		shaderParam.pointSizeX = float(m_pointSize / (tanY * aspectRatio));
		shaderParam.pointSizeY = float(m_pointSize / tanY);
	}

	shaderParam.scannerPosition = YmVectorUtil::StaticCast<XMFLOAT3>(m_scannerPosition);
	shaderParam.scannerDistanceUpperBound = (float)m_scannerDistanceUpperBound;
	shaderParam.scannerDistanceDepthOffset = (float)m_scannerDistanceDepthOffset;
	shaderParam.isUseScannerPosition = m_isUseScannerPosition;

	SetConstantBufferData(m_pShaderParamConstBuf, shaderParam);
	m_isNeedUpdateShaderParam = false;
}

double YmTngnShaderImpl::GetAspectRatio() const
{
	double aspectRatio = 1;
	if (0 < m_viewSize[0] && 0 < m_viewSize[1])
	{
		aspectRatio = double(m_viewSize[0]) / m_viewSize[1];
	}
	return aspectRatio;
}

XMMATRIX YmTngnShaderImpl::GetProjectionMatrix(double aspectRatio) const
{
	return XMMatrixPerspectiveFovRH(XMConvertToRadians((float)m_fovAngleYDeg), (float)aspectRatio, (float)m_viewNearZ, (float)m_viewFarZ);
}

static int CastFloatToInt(float f)
{
	if (f < 0) {
		return -static_cast<int>(floor(-f + 0.5));
	}
	else {
		return static_cast<int>(floor(f + 0.5));
	}
}

bool YmTngnShaderImpl::ConvertGlobalCoordToViewIntCoord(const YmVector3d& global, YmVector2i* pViewCoord) const
{
	XMMATRIX globalToProjMatrix = XMMatrixMultiply(m_pViewOp->GetViewMatrix(), GetProjectionMatrix(GetAspectRatio()));
	YmVector3f globalF = YmVectorUtil::StaticCast<YmVector3f>(global);
	XMVECTOR viewCoordHomo = XMVector3Transform(XMVectorSet(globalF[0], globalF[1], globalF[2], 1.0f), globalToProjMatrix);
	float w = XMVectorGetW(viewCoordHomo);
	const double tolZero = 1e-6;
	if (w < tolZero) {
		return false;
	}
	if (pViewCoord != nullptr) {
		YmVector2i viewSize = GetViewSize();
		*pViewCoord = YmVectorUtil::Make(
			CastFloatToInt((XMVectorGetX(viewCoordHomo) / w + 1) / 2 * viewSize[0]),
			CastFloatToInt((1 - XMVectorGetY(viewCoordHomo) / w) / 2 * viewSize[1])
		);
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnShaderImpl::InitializeShaderContextsForNormalRendering()
{
	const YmTString hlslFilePath = GetHslsFilePath(_T("DefaultShader.hlsl"));
	const D3D_SHADER_MACRO aMacro[] = {
		{ nullptr, nullptr }
	};

	D3D11_INPUT_ELEMENT_DESC aPointListElem[] = {
		{ "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	0,	D3D11_INPUT_PER_VERTEX_DATA,	0},
		{ "COLOR"	,	0,	DXGI_FORMAT_R8G8B8A8_UNORM,	    0,	12,	D3D11_INPUT_PER_VERTEX_DATA,	0},
	};
	m_pointListSc.Init(
		CreateInputLayout(aPointListElem, sizeof(aPointListElem) / sizeof(aPointListElem[0]), hlslFilePath, "vsMain", aMacro),
		CreateVertexShader(hlslFilePath, "vsMain", aMacro),
		m_pShaderParamConstBuf,
		CreateGeometryShader(hlslFilePath, "gsMain", aMacro),
		m_pShaderParamConstBuf,
		CreatePixelShader(hlslFilePath, "psMain", aMacro), nullptr
	);
}

void YmTngnShaderImpl::InitializeShaderContextsForPickableRendering()
{
	const YmTString hlslFilePath = GetHslsFilePath(_T("DefaultShader.hlsl"));
	const D3D_SHADER_MACRO aMacro[] = {
		{ "PICKABLE_MODE", "1" },
		{ nullptr, nullptr }
	};

	D3D11_INPUT_ELEMENT_DESC aPointListElem[] = {
		{ "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	0,	D3D11_INPUT_PER_VERTEX_DATA,	0},
		{ "COLOR"	,	0,	DXGI_FORMAT_R8G8B8A8_UNORM,	    0,	12,	D3D11_INPUT_PER_VERTEX_DATA,	0},
		{ "PICK_ID"	,	0,	DXGI_FORMAT_R16G16B16A16_UINT,	1,	0,	D3D11_INPUT_PER_VERTEX_DATA,	0},
	};
	m_pickablePointListSc.Init(
		CreateInputLayout(aPointListElem, sizeof(aPointListElem) / sizeof(aPointListElem[0]), hlslFilePath, "vsMain", aMacro),
		CreateVertexShader(hlslFilePath, "vsMain", aMacro),
		m_pShaderParamConstBuf,
		CreateGeometryShader(hlslFilePath, "gsMain", aMacro),
		m_pShaderParamConstBuf,
		CreatePixelShader(hlslFilePath, "psMain", aMacro), nullptr
	);
}

void YmTngnShaderImpl::InitializeShaderContextsForTriangleListNormalRendering()
{
	const YmTString hlslFilePath = GetHslsFilePath(_T("TriangleListShader.hlsl"));
	const D3D_SHADER_MACRO aMacro[] = {
		{ nullptr, nullptr }
	};

	D3D11_INPUT_ELEMENT_DESC aPointListElem[] = {
		{ "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	0,	D3D11_INPUT_PER_VERTEX_DATA,	0},
		{ "NORMAL",		0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	12,	D3D11_INPUT_PER_VERTEX_DATA,	0},
		{ "COLOR"	,	0,	DXGI_FORMAT_R8G8B8A8_UNORM,	    0,	24,	D3D11_INPUT_PER_VERTEX_DATA,	0},
	};
	m_triangleListSc.Init(
		CreateInputLayout(aPointListElem, sizeof(aPointListElem) / sizeof(aPointListElem[0]), hlslFilePath, "vsMain", aMacro),
		CreateVertexShader(hlslFilePath, "vsMain", aMacro),
		m_pShaderParamConstBuf,
		nullptr, nullptr,
		CreatePixelShader(hlslFilePath, "psMain", aMacro), nullptr
	);
}

void YmTngnShaderImpl::InitializeShaderContextsForLineListNormalRendering()
{
	const YmTString hlslFilePath = GetHslsFilePath(_T("LineListShader.hlsl"));
	const D3D_SHADER_MACRO aMacro[] = {
		{ nullptr, nullptr }
	};

	D3D11_INPUT_ELEMENT_DESC aPointListElem[] = {
		{ "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	0,	D3D11_INPUT_PER_VERTEX_DATA,	0},
		{ "COLOR"	,	0,	DXGI_FORMAT_R8G8B8A8_UNORM,	    0,	12,	D3D11_INPUT_PER_VERTEX_DATA,	0},
	};
	m_lineListSc.Init(
		CreateInputLayout(aPointListElem, sizeof(aPointListElem) / sizeof(aPointListElem[0]), hlslFilePath, "vsMain", aMacro),
		CreateVertexShader(hlslFilePath, "vsMain", aMacro),
		m_pShaderParamConstBuf,
		nullptr, nullptr,
		CreatePixelShader(hlslFilePath, "psMain", aMacro), nullptr
	);
}

YmTString YmTngnShaderImpl::GetHslsFilePath(const YmTString& fileName)
{
	HMODULE hModule = GetModuleHandle(nullptr);
	TCHAR exeFilePath[_MAX_PATH];
	DWORD nChar = ::GetModuleFileName(hModule, exeFilePath, _MAX_PATH);
	if (_MAX_PATH <= nChar) {
		YM_THROW_ERROR("Module path is too long.");
	}
	return YmFilePath(exeFilePath).GetParentDirectory().AppendFileName(fileName).ToTString();
}

D3DBufferPtr YmTngnShaderImpl::CreateConstantBuffer(size_t nByte)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = ((static_cast<UINT>(nByte) + 15) / 16) * 16;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	YM_IS_TRUE(bd.ByteWidth <= D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT);

	D3DBufferPtr pBuffer;
	HRESULT hr = m_pDevice->CreateBuffer(&bd, nullptr, &pBuffer);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateBuffer");
	}

	return pBuffer;
}

void YmTngnShaderImpl::SetShaderContext(const YmTngnShaderContext& context)
{
	ID3D11Buffer* apBuffer[1] = { nullptr };
	m_pDc->IASetInputLayout(context.GetIAInputLayout().Get());

	m_pDc->VSSetShader(context.GetVertexShader().Get(), nullptr, 0);
	apBuffer[0] = { context.GetVSConstantBuffer().Get() };
	m_pDc->VSSetConstantBuffers(0, 1, apBuffer);

	m_pDc->GSSetShader(context.GetGeometryShader().Get(), nullptr, 0);
	apBuffer[0] = { context.GetGSConstantBuffer().Get() };
	m_pDc->GSSetConstantBuffers(0, 1, apBuffer);

	m_pDc->PSSetShader(context.GetPixelShader().Get(), nullptr, 0);
	apBuffer[0] = { context.GetPSConstantBuffer().Get() };
	m_pDc->PSSetConstantBuffers(0, 1, apBuffer);
}

////////////////////////////////////////////////////////////////////////////////

D3DInputLayoutPtr YmTngnShaderImpl::CreateInputLayout(
	const D3D11_INPUT_ELEMENT_DESC* aElement, UINT nElement,
	const YmTString& fileName, const string& entryPoint, const D3D_SHADER_MACRO* aMacro
)
{
	bool showError = true;

#if defined(_DEBUG)
	// グラフィックデバッグツールによるシェーダーのデバッグを有効にする
	UINT	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT	compileFlags = 0;
#endif
	D3DBlobPtr blob;
	D3DBlobPtr pErrorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(
		CT2W(fileName.c_str()), aMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), VS_COMPILE_TARGET, compileFlags, 0,
		&blob, &pErrorBlob
	);
	if (FAILED(hr))
	{
		if (showError) {
			if (pErrorBlob)
			{
				MessageBox(nullptr, CA2T((char*)pErrorBlob->GetBufferPointer()), _T(""), 0);
			}
		}
		YM_IGNORE_ERROR("D3DCompileFromFile");
		return nullptr;
	}

	D3DInputLayoutPtr pLayout;
	hr = m_pDevice->CreateInputLayout(aElement, nElement, blob->GetBufferPointer(),
		blob->GetBufferSize(), &pLayout);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateInputLayout");
	}

	return pLayout;
}

D3DVertexShaderPtr YmTngnShaderImpl::CreateVertexShader(
	const YmTString& fileName, const std::string& entryPoint, const D3D_SHADER_MACRO* aMacro
)
{
	bool showError = true;
	D3DVertexShaderPtr pShader;

#if defined(_DEBUG)
	UINT	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT	compileFlags = 0;
#endif
	D3DBlobPtr blob;
	//setlocale(LC_CTYPE, "jpn");
	D3DBlobPtr pErrorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(CT2W(fileName.c_str()), aMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), VS_COMPILE_TARGET, compileFlags, 0, &blob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (showError)
		{
			if (pErrorBlob)
			{
				MessageBox(nullptr, CA2T((char*)pErrorBlob->GetBufferPointer()), _T(""), 0);
			}
		}
		YM_IGNORE_ERROR("D3DCompileFromFile");
		return nullptr;
	}

	hr = m_pDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateVertexShader");
	}

	return pShader;
}

D3DGeometryShaderPtr YmTngnShaderImpl::CreateGeometryShader(
	const YmTString& fileName, const string& entryPoint, const D3D_SHADER_MACRO* aMacro
)
{
	bool showError = true;
	D3DGeometryShaderPtr pShader;

#if defined(_DEBUG)
	UINT	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT	compileFlags = 0;
#endif
	D3DBlobPtr blob;
	D3DBlobPtr pErrorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(CT2W(fileName.c_str()), aMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), "gs_5_0", compileFlags, 0, &blob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (showError) {
			if (pErrorBlob != nullptr)
			{
				MessageBox(nullptr, CA2T((char*)pErrorBlob->GetBufferPointer()), _T(""), 0);
			}
		}
		YM_IGNORE_ERROR("D3DCompileFromFile");
		return nullptr;
	}

	hr = m_pDevice->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateGeometryShader");
	}

	return pShader;
}

D3DPixelShaderPtr YmTngnShaderImpl::CreatePixelShader(
	const YmTString& fileName, const string& entryPoint, const D3D_SHADER_MACRO* aMacro
)
{
	bool showError = true;
	D3DPixelShaderPtr pShader;

#if defined(_DEBUG)
	UINT	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT	compileFlags = 0;
#endif
	D3DBlobPtr blob;
	D3DBlobPtr pErrorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(CT2W(fileName.c_str()), aMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), "ps_5_0", compileFlags, 0, &blob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (showError) {
			if (pErrorBlob != nullptr)
			{
				MessageBox(nullptr, CA2T((char*)pErrorBlob->GetBufferPointer()), _T(""), 0);
			}
		}
		YM_IGNORE_ERROR("D3DCompileFromFile");
		return nullptr;
	}

	hr = m_pDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pShader);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreatePixelShader");
	}

	if (false) {
		DumpShaderReflection("PixelShader", blob);
	}

	return pShader;
}

void YmTngnShaderImpl::DumpShaderReflection(const char* pHeader, const D3DBlobPtr& blob)
{
	YM_NOEXCEPT_BEGIN("YmTngnShaderImpl::DumpShaderReflection");
	YmDebugOutputStream dbg;
	if (pHeader) {
		dbg << "[[[" << pHeader << "]]]" << endl;
	}

	YmComPtr<ID3D11ShaderReflection> pReflection;
	HRESULT hr = D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&pReflection));
	if (FAILED(hr)) {
		YM_THROW_ERROR("D3DReflect");
	}

	D3D11_SHADER_DESC shaderdesc;
	pReflection->GetDesc(&shaderdesc);

	// analyze input parameter
	if (0 > shaderdesc.InputParameters) {
		dbg << " [InputParameter]" << endl;

		for (UINT i = 0; i < shaderdesc.InputParameters; ++i) {
			D3D11_SIGNATURE_PARAMETER_DESC sigDesc;
			pReflection->GetInputParameterDesc(i, &sigDesc);
			dbg << " (" << i << ": " << sigDesc.SemanticName << ") (" << sigDesc.SemanticIndex  << ", " << sigDesc.SemanticIndex << ")"
				<< sigDesc.ComponentType << " " << sigDesc.Mask << endl;
		}
	}

	// analize constant buffer
	for (UINT i = 0; i < shaderdesc.ConstantBuffers; ++i) {
		auto buffer = pReflection->GetConstantBufferByIndex(i);
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		buffer->GetDesc(&bufferDesc);
		dbg << " [" << i << ": " << bufferDesc.Name << "]" << endl;

		for (UINT j = 0; j < bufferDesc.Variables; ++j) {
			auto variable = buffer->GetVariableByIndex(j);
			D3D11_SHADER_VARIABLE_DESC variableDesc;
			variable->GetDesc(&variableDesc);
			dbg << "  (" << j << ") " << variableDesc.Name << " " << variableDesc.StartOffset << " " << variableDesc.Size << endl;
		}
	}
	YM_NOEXCEPT_END;
}

////////////////////////////////////////////////////////////////////////////////
