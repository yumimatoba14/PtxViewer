#include "pch.h"
#include "YmTngnDmPtxFiles.h"
#include "YmPtxFileParser.h"
#include "YmTngnDraw.h"
#include "YmTngnDmPointBlockListFile.h"
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

static shared_ptr<YmTngnDmPointBlockList> ReadPtxFileImpl(const char* pPtxFilePath)
{
	auto pOutputBuf = make_unique<YmWin32FileBuf>();
	pOutputBuf->OpenTempFile();
	YmPtxFileParser::FileHeader header;
	{
		YmTngnPointBlockListBuilder builder(pOutputBuf.get());
		YmOrtho3dXform<double> localToGlobal;
		bool isFirstPoint = true;
		auto onParsePoint = [&](const YmPtxFileParser::FileHeader& header, int64_t col, int64_t row, const YmPtxFileParser::PointData& point) {
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

	// isImageReliable represents whether this point set would construct a image.
	const int64_t pixelCountLowerBound = 16;
	bool isImageReliable = pixelCountLowerBound < header.numberOfColumn && pixelCountLowerBound < header.numberOfRow;
	if (isImageReliable) {
		double aspectRatio = 5;
		if (header.numberOfColumn * aspectRatio < header.numberOfRow || header.numberOfRow * aspectRatio < header.numberOfColumn) {
			isImageReliable = false;
		}
	}

	auto pModel = make_shared<YmTngnDmPointBlockListFile>(move(pOutputBuf));
	if (isImageReliable) {
		// Rely on the scanner position when this ptx constructs an image.
		pModel->SetScannerPosition(header.scannerOrg);
	}
	return pModel;
}

YmTString YmTngnDmPtxFiles::ReadPtxFile(const YmTString& filePath)
{
	auto pModel = ReadPtxFileImpl(ATL::CT2A(filePath.c_str()));
	Content content = { filePath, true, pModel };
	m_contents.push_back(content);
	m_blockListImpl.AddInstances(*pModel);
	return filePath;
}

void YmTngnDmPtxFiles::OnDraw(YmTngnDraw* pDraw)
{
	m_blockListImpl.Draw(pDraw);
}

////////////////////////////////////////////////////////////////////////////////
