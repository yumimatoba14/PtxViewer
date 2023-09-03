#include "pch.h"
#include "YmTngnDmPointBlockList.h"
#include "YmTngnDraw.h"
#include "YmBase/YmBinaryFormatter.h"

using namespace std;
using namespace DirectX;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmTngnDmPointBlockList::YmTngnDmPointBlockList(std::unique_ptr<YmWin32FileBuf> pBaseFile)
	: m_pBaseFile(move(pBaseFile))
{
	YM_IS_TRUE(m_pBaseFile);
	m_mmFile.AttachFileToRead(m_pBaseFile->GetHandle());
}

YmTngnDmPointBlockList::~YmTngnDmPointBlockList()
{
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmPointBlockList::SetScannerPosition(const YmVector3d& scannerPos)
{
	PrepareBlockData();

	for (auto& inst : m_instanceList) {
		inst.pPointBlock->SetScannerPosition(scannerPos);
	}
}

void YmTngnDmPointBlockList::PrepareBlockData()
{
	if (!m_instanceList.empty()) {
		return;
	}

	YM_IS_TRUE(m_mmFile.IsOpened());
	m_pBaseFile->pubseekpos(0);

	YmTngnModel::FileHeader fileHeader;
	fileHeader.ReadFrom(m_pBaseFile.get());
	if (YmTngnModel::CURRENT_FILE_VERSION < fileHeader.version) {
		YM_THROW_ERROR("This file is newer than this module.");
	}

	YmBinaryFormatter input(m_pBaseFile.get());
	input.SetFormatFlags(fileHeader.formatterBitFlags);

	input.SetCurrentPosition(fileHeader.contentPosition);
	size_t nBlock = static_cast<size_t>(input.ReadInt32());
	m_instanceList.reserve(nBlock);
	for (size_t iBlock = 0; iBlock < nBlock; ++iBlock) {
		YmTngnModel::PointBlockHeader blockHeader;
		input.ReadBytes(reinterpret_cast<char*>(&blockHeader), sizeof(YmTngnModel::PointBlockHeader));

		InstanceData instance;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				instance.localToGlobalMatrix.m[i][j] =
					static_cast<float>(blockHeader.localToGlobalMatrix[i*4 + j]);
			}
		}

		instance.aabb.Extend(blockHeader.aAabbPoint[0]);
		instance.aabb.Extend(blockHeader.aAabbPoint[1]);
		instance.pPointBlock = make_unique<YmTngnDmExclusiveLodPointList>(m_mmFile, blockHeader.firstBytePos);
		m_instanceList.push_back(move(instance));
	}
}

////////////////////////////////////////////////////////////////////////////////

static double CalcPointListEnumerationPrecision(
	const XMFLOAT4X4& modelToViewMatrix, const YmAabBox3d& aabb, double distanceLBIn
)
{
	const double distanceLB = max(0, distanceLBIn);
	double minDistance = 1e128;
	for (int i = 0; i < 8; ++i) {
		YmVector3d pnt;
		pnt[0] = ((i & 0x01) == 0 ? aabb.GetMinPoint()[0] : aabb.GetMaxPoint()[0]);
		pnt[1] = ((i & 0x02) == 0 ? aabb.GetMinPoint()[1] : aabb.GetMaxPoint()[1]);
		pnt[2] = ((i & 0x04) == 0 ? aabb.GetMinPoint()[2] : aabb.GetMaxPoint()[2]);
		double value = modelToViewMatrix.m[2][3];
		for (int j = 0; j < 3; ++j) {
			value += modelToViewMatrix.m[2][j] * pnt[j];
		}
		value *= -1;
		if (value < minDistance) {
			minDistance = value;
			if (minDistance < distanceLB) {
				minDistance = distanceLB;
				break;
			}
		}
	}
	return  0.001 * minDistance;  // TODO: fix me
}

void YmTngnDmPointBlockList::OnDraw(YmTngnDraw* pDraw)
{
	PrepareBlockData();
	const int64_t maxDrawnPointCountPerFrame = 1 << 20;

	if (!pDraw->IsProgressiveViewFollowingFrame()) {
		UpdateDrawnInstances(pDraw);

		XMFLOAT4X4 modelToViewMatrix;
		XMStoreFloat4x4(&modelToViewMatrix, pDraw->GetModelToViewMatrix());
		size_t nDrawnInst = m_drawnInstanceIndices.size();
		int64_t maxPointPerInst = 1 << 20;
		const int numInstMax = 3;
		if (0 < nDrawnInst && nDrawnInst < numInstMax) {
			if (nDrawnInst == 1) {
				maxPointPerInst = maxDrawnPointCountPerFrame;
			}
			else {
				maxPointPerInst = maxDrawnPointCountPerFrame / nDrawnInst;
			}
		}
		else {
			maxPointPerInst = maxDrawnPointCountPerFrame / numInstMax;
		}

		double persNearZ = pDraw->GetPerspectiveViewNearZ();
		for (auto iBlock : m_drawnInstanceIndices) {
			const InstanceData& instance = m_instanceList[iBlock];
			pDraw->SetModelMatrix(instance.localToGlobalMatrix);
			double precision = CalcPointListEnumerationPrecision(modelToViewMatrix, instance.aabb, persNearZ);
			instance.pPointBlock->SetDrawingPrecision(precision);
			instance.pPointBlock->SetMaxPointCountDrawnPerFrame(maxPointPerInst);
			//instance.pPointBlock->SetPointSelectionTargetIdFirst(uint64_t(iBlock + 1) << 32);
			instance.pPointBlock->PrepareFirstDraw(pDraw);
		}
	}
	bool isProgressiveMode = pDraw->IsProgressiveViewMode();

	if (isProgressiveMode) {
#if 1
		DrawInstancesInProgressiveMode(pDraw, maxDrawnPointCountPerFrame);
#else
		for (auto iBlock : m_drawnInstanceIndices) {
			const InstanceData& instance = m_instanceList[iBlock];
			g.SetModelMatrix(instance.localToGlobalMatrix);
			instance.pObject->DrawTo(g);
			if (isProgressiveMode && maxDrawnPointCountPerFrame < g.GetDrawnPointCount()) {
				break;
			}
		}
#endif
	}
	else {
		for (auto iBlock : m_drawnInstanceIndices) {
			const InstanceData& instance = m_instanceList[iBlock];
			pDraw->SetModelMatrix(instance.localToGlobalMatrix);
			instance.pPointBlock->Draw(pDraw);
		}
	}
#if 0
	for (auto& inst : m_instanceList) {
		pDraw->SetModelMatrix(inst.localToGlobalMatrix);
		inst.pPointBlock->Draw(pDraw);
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////

static bool CalcBoxDistanceInProjection(const XMMATRIX& modelProjMatrix, const YmAabBox3d& aabb, double* pDistance)
{
	const float tolZero = 1e-6f;
	*pDistance = 0;
	YmAabBox3d projAabb;
	bool hasNonPositiveW = false;
	for (int i = 0; i < 8; ++i) {
		float coord[3];
		coord[0] = static_cast<float>((i & 0x01) ? aabb.GetMaxPoint()[0] : aabb.GetMinPoint()[0]);
		coord[1] = static_cast<float>((i & 0x02) ? aabb.GetMaxPoint()[1] : aabb.GetMinPoint()[1]);
		coord[2] = static_cast<float>((i & 0x04) ? aabb.GetMaxPoint()[2] : aabb.GetMinPoint()[2]);

		XMVECTOR moelVec = XMVectorSet(coord[0], coord[1], coord[2], 1);
		XMVECTOR projVecHomo = XMVector4Transform(moelVec, modelProjMatrix);
		float w = XMVectorGetW(projVecHomo);
		// w = -zv where zv is z in view coordinate system. Negative z shall be drawn in RH system.
		if (w < tolZero) {
			if (0 <= XMVectorGetZ(projVecHomo)) {
				XMVectorSetZ(projVecHomo, 0.0f);
			}
			w = tolZero;
			hasNonPositiveW = true;
		}
		double invW = 1.0 / w;
		YmVector3d projVec;
		projVec[0] = XMVectorGetX(projVecHomo) * invW;
		projVec[1] = XMVectorGetY(projVecHomo) * invW;
		projVec[2] = XMVectorGetZ(projVecHomo) * invW;
		projAabb.Extend(projVec);
	}

	YmVector3d minPoint = projAabb.GetMinPoint();
	YmVector3d maxPoint = projAabb.GetMaxPoint();
	for (int i = 0; i < 2; ++i) {
		if (maxPoint[i] < -1 || 1 < minPoint[i]) {
			return false;
		}
	}
	if (maxPoint[2] < 0 || 1 < minPoint[2]) {
		return false;
	}
	if (!hasNonPositiveW && 0 < minPoint[2]) {
		*pDistance = minPoint[2];
	}
	return true;
}

void YmTngnDmPointBlockList::UpdateDrawnInstances(YmTngnDraw* pDraw)
{
	m_drawnInstanceIndices.clear();

	XMMATRIX modelProjMatrix = pDraw->GetModelToProjectionMatrix();

	size_t nBlock = m_instanceList.size();
	multimap<double, size_t> distanceToBlock;
	for (size_t iBlock = 0; iBlock < nBlock; ++iBlock) {
		double distance = 0;
		bool isVisible = CalcBoxDistanceInProjection(modelProjMatrix, m_instanceList[iBlock].aabb, &distance);
		if (!isVisible) {
			continue;
		}
		distanceToBlock.emplace(distance, iBlock);
	}
	for (auto distToIndex : distanceToBlock) {
		m_drawnInstanceIndices.push_back(distToIndex.second);
	}
}

void YmTngnDmPointBlockList::DrawInstancesInProgressiveMode(
	YmTngnDraw* pDraw, int64_t maxDrawnPointCountPerFrame
)
{
	size_t numDrawingInstance = 0;
	for (auto iBlock : m_drawnInstanceIndices) {
		const InstanceData& instance = m_instanceList[iBlock];
		if (!instance.pPointBlock->IsDrawingEnded()) {
			++numDrawingInstance;
		}
	}

	while (0 < numDrawingInstance) {
		for (auto iBlock : m_drawnInstanceIndices) {
			const InstanceData& instance = m_instanceList[iBlock];
			if (instance.pPointBlock->IsDrawingEnded()) {
				continue;
			}
			pDraw->SetModelMatrix(instance.localToGlobalMatrix);
			instance.pPointBlock->DrawAfterPreparation(pDraw);
			if (maxDrawnPointCountPerFrame < pDraw->GetDrawnPointCount()) {
				return;
			}
			if (instance.pPointBlock->IsDrawingEnded()) {
				--numDrawingInstance;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
