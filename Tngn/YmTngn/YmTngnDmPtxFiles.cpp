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

YmTngnDmPtxFiles::YmTngnDmPtxFiles() : YmTngnDmPtxFiles(YmTngnViewConfig())
{
}

YmTngnDmPtxFiles::YmTngnDmPtxFiles(const YmTngnViewConfig& config)
	: m_config(config)
{
	m_blockListImpl.SetMaxDrawnPointCountPerFrame(
		m_config.GetDoubleValueAsInt64(YmTngnViewConfig::DM_PTX_FILE_MAX_DRAWN_POINT_PER_FRAME_MB, 1 << 20)
	);
	// Draw point block without using scanner position at last.
	m_blockListImpl.SetScannerDistanceForPointBlockWithoutScanerPos(
		m_config.GetDoubleValue(YmTngnViewConfig::SCANNER_DISTANCE_UB)
	);
}

YmTngnDmPtxFiles::~YmTngnDmPtxFiles()
{
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmPtxFiles::SetDrawWithScannerPosition(bool isUse)
{
	m_isDrawWithScannerPosition = isUse;
	m_blockListImpl.SetUseScannerPosition(m_isDrawWithScannerPosition);
}

static int MakeRgb(const int rgb[3])
{
	return rgb[0] | (rgb[1] << 8) | (rgb[2] << 16);
}

static shared_ptr<YmTngnDmPointBlockList> ReadPtxFileImpl(const YmTngnViewConfig& config, const char* pPtxFilePath)
{
	auto pOutputBuf = make_unique<YmWin32FileBuf>();
	pOutputBuf->OpenTempFile();
	bool isImageReliable = false;
	{
		YmTngnPointBlockListBuilder builder(pOutputBuf.get());
		builder.SetTargetPointCountPerBlock(
			config.GetDoubleValueAsInt64(YmTngnViewConfig::DM_PTX_FILE_POINT_COUNT_PER_BLOCK, 1024*1024)
		);
		YmOrtho3dXform<double> localToGlobal;
		bool isFirstPoint = true;
		auto onParsePoint = [&](const YmPtxFileParser::FileHeader& h, int64_t col, int64_t row, const YmPtxFileParser::PointData& point) {
			if (isFirstPoint) {
				isFirstPoint = false;
				localToGlobal = YmOrtho3dXform<double>::MakeFromXy(
					h.scannerOrg, h.scannerDirX, h.scannerDirY
				);
			}
			YmTngnModel::PointType tngnPoint;
			tngnPoint.coord = YmVectorUtil::StaticCast<YmVector3f>(localToGlobal.ConvertCoord(point.localPoint));
			tngnPoint.rgba = YmRgba4b(point.rgb[0], point.rgb[1], point.rgb[2]);
			builder.AddPoint(tngnPoint);
		};

		YmPtxFileParser parser;
		parser.SetRadiusLowerBound(config.GetDoubleValue(YmTngnViewConfig::DM_PTX_FILE_RADIUS_LOWER_BOUND));
		if (0 < config.GetDoubleValue(YmTngnViewConfig::DM_PTX_FILE_RADIUS_UPPER_BOUND)) {
			parser.SetRadiusUpperBound(config.GetDoubleValue(YmTngnViewConfig::DM_PTX_FILE_RADIUS_UPPER_BOUND));
		}
		YmPtxFileParser::FileHeader header = parser.ParseFile(pPtxFilePath, onParsePoint);

		// isImageReliable represents whether this point set would construct a image.
		const int64_t pixelCountLowerBound = 16;
		isImageReliable = pixelCountLowerBound < header.numberOfColumn&& pixelCountLowerBound < header.numberOfRow;
		if (isImageReliable) {
			double aspectRatio = 5;
			if (header.numberOfColumn * aspectRatio < header.numberOfRow || header.numberOfRow * aspectRatio < header.numberOfColumn) {
				isImageReliable = false;
			}
		}

		if (isImageReliable) {
			// Rely on the scanner position when this ptx constructs an image.
			builder.SetScannerPosition(header.scannerOrg);
		}

		builder.BuildPointBlockFile();
	}


	auto pModel = make_shared<YmTngnDmPointBlockListFile>(move(pOutputBuf));
	return pModel;
}

YmTString YmTngnDmPtxFiles::ReadPtxFile(const YmTString& filePath)
{
	auto pModel = ReadPtxFileImpl(m_config, ATL::CT2A(filePath.c_str()));
	pModel->SetUseScannerPosition(IsDrawWithScannerPosition());
	Content content = { filePath, true, pModel };
	m_contents.push_back(content);
	m_blockListImpl.AddInstances(*pModel);
	return filePath;
}

////////////////////////////////////////////////////////////////////////////////

bool YmTngnDmPtxFiles::OnSetPickEnabled(bool bEnable)
{
	return m_blockListImpl.SetPickEnabled(bEnable);
}

void YmTngnDmPtxFiles::OnDraw(YmTngnDraw* pDraw)
{
	m_blockListImpl.Draw(pDraw);
}

std::vector<YmTngnPointListVertex> YmTngnDmPtxFiles::OnFindPickedPoints(YmTngnPickTargetId id)
{
	return m_blockListImpl.FindPickedPoints(id);
}

////////////////////////////////////////////////////////////////////////////////
