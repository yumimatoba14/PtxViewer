#pragma once

#include "YmBase/YmWin32.h"
#include "YmTngnModel/YmTngnModelFwd.h"
#include <d3d11.h>
#pragma warning(push)
#pragma warning(disable: 4793)
#include <DirectXMath.h>
#pragma warning(pop)
//#include <wrl/client.h>	// WRL is not available on CLR environment.
#include "YmComPtr.h"
#define _USE_MATH_DEFINES	// In order to use M_PI, which is not in the C standard.
#include <math.h>

namespace Ymcpp {

class YmTngnDraw;
class YmTngnDrawingModel;
struct YmTngnPointListVertex;
struct YmTngnTriangleVertex;
class YmTngnViewConfig;
class YmViewOp;

using YmTngnDrawingModelPtr = std::shared_ptr<YmTngnDrawingModel>;
using YmTngnPickTargetId = uint64_t;
#define YM_TNGN_PICK_TARGET_NULL YmTngnPickTargetId(0)

using DXGIFactoryPtr = YmComPtr<IDXGIFactory>;
using DXGIOutputPtr = YmComPtr<IDXGIOutput>;
using DXGIAdapterPtr = YmComPtr<IDXGIAdapter>;

using D3DDevicePtr = YmComPtr<ID3D11Device> ;
using D3DDeviceContextPtr = YmComPtr<ID3D11DeviceContext>;
using DXGISwapChainPtr = YmComPtr<IDXGISwapChain>;
using D3DBlendStatePtr = YmComPtr<ID3D11BlendState>;
using D3DDepthStencilStatePtr = YmComPtr<ID3D11DepthStencilState>;
using D3DRasterizerStatePtr = YmComPtr<ID3D11RasterizerState>;

using D3DInputLayoutPtr = YmComPtr<ID3D11InputLayout>;
using D3DVertexShaderPtr = YmComPtr<ID3D11VertexShader>;
using D3DGeometryShaderPtr = YmComPtr<ID3D11GeometryShader>;
using D3DPixelShaderPtr = YmComPtr<ID3D11PixelShader>;

using D3DDepthStencilViewPtr = YmComPtr<ID3D11DepthStencilView>;
using D3DRenderTargetViewPtr = YmComPtr<ID3D11RenderTargetView>;
using D3DTexture2DPtr = YmComPtr<ID3D11Texture2D>;
using D3DResourcePtr = YmComPtr<ID3D11Resource>;
using D3DBufferPtr = YmComPtr<ID3D11Buffer>;
using D3DBlobPtr = YmComPtr< ID3DBlob>;


struct YmDx11BufferWithSize
{
	D3DBufferPtr pBuffer;
	UINT nBufferByte;
};
}

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
