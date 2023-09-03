#include "pch.h"
#include "YmTngnDmPtxFiles.h"
#include "YmPtxFileParser.h"
#include "YmTngnDraw.h"
#include "YmTngnDmMemoryPointList.h"
#include "YmTngnDmPointBlockList.h"
#include "YmTngnModel/YmTngnPointBlockListBuilder.h"
#include "YmBase/YmOrtho3dXform.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmTngnDmPtxFiles::YmTngnDmPtxFiles()
{
}

YmTngnDmPtxFiles::~YmTngnDmPtxFiles()
{
}

////////////////////////////////////////////////////////////////////////////////

static int MakeRgb(const int rgb[3])
{
	return rgb[0] | (rgb[1] << 8) | (rgb[2] << 16);
}

static YmTngnDrawingModelPtr ReadPtxFileImpl(const char* pPtxFilePath)
{
	using namespace std;
	vector<YmTngnPointListVertex> vertices;
	YmOrtho3dXform<double> localToGlobal;
	bool isFirstPoint = true;
	auto onParsePoint = [&](const YmPtxFileParser::FileHeader& header, size_t col, size_t row, const YmPtxFileParser::PointData& point) {
		if (isFirstPoint) {
			isFirstPoint = false;
			localToGlobal = YmOrtho3dXform<double>::MakeFromXy(
				header.scannerOrg, header.scannerDirX, header.scannerDirY
			);
		}
		YmTngnPointListVertex vertex;
		vertex.position = YmVectorUtil::StaticCast<DirectX::XMFLOAT3>(localToGlobal.ConvertCoord(point.localPoint));
		vertex.rgba = MakeRgb(point.rgb);
		vertices.push_back(vertex);
	};

	YmPtxFileParser parser;
	parser.SetRadiusLowerBound(0.001);
	auto header = parser.ParseFile(pPtxFilePath, onParsePoint);

	auto pModel = make_shared<YmTngnDmMemoryPointList>(move(vertices));
	pModel->SetScannerPosition(header.scannerOrg);
	return pModel;
}

static YmTngnDrawingModelPtr ReadPtxFileImpl2(const char* pPtxFilePath)
{
	auto pOutputBuf = make_unique<YmWin32FileBuf>();
	pOutputBuf->OpenTempFile();
	YmPtxFileParser::FileHeader header;
	{
		YmTngnPointBlockListBuilder builder(pOutputBuf.get());
		YmOrtho3dXform<double> localToGlobal;
		bool isFirstPoint = true;
		auto onParsePoint = [&](const YmPtxFileParser::FileHeader& header, size_t col, size_t row, const YmPtxFileParser::PointData& point) {
			if (isFirstPoint) {
				isFirstPoint = false;
				localToGlobal = YmOrtho3dXform<double>::MakeFromXy(
					header.scannerOrg, header.scannerDirX, header.scannerDirY
				);
			}
			YmTngnModel::PointType tngnPoint;
			tngnPoint.coord = YmVectorUtil::StaticCast<YmVector3f>(localToGlobal.ConvertCoord(point.localPoint));
			tngnPoint.rgba = YmRgba4b(point.rgb[0], point.rgb[1], point.rgb[2]);
			builder.AddPoint(tngnPoint);
		};

		YmPtxFileParser parser;
		parser.SetRadiusLowerBound(0.001);
		header = parser.ParseFile(pPtxFilePath, onParsePoint);

		builder.BuildPointBlockFile();
	}

	auto pModel = make_shared<YmTngnDmPointBlockList>(move(pOutputBuf));
	pModel->SetScannerPosition(header.scannerOrg);
	return pModel;
}

YmTString YmTngnDmPtxFiles::ReadPtxFile(const YmTString& filePath)
{
	auto pModel = ReadPtxFileImpl2(ATL::CT2A(filePath.c_str()));
	Content content = { filePath, true, pModel };
	m_contents.push_back(content);
	return filePath;
}

void YmTngnDmPtxFiles::AddChildModel(const YmTString& name, const YmTngnDrawingModelPtr& pModel)
{
	Content content = { name, true, pModel };
	m_contents.push_back(content);
}

void YmTngnDmPtxFiles::OnDraw(YmTngnDraw* pDraw)
{
	for (auto& content : m_contents) {
		if (!content.isActive) {
			continue;
		}
		content.pModel->Draw(pDraw);
	}
}

////////////////////////////////////////////////////////////////////////////////
