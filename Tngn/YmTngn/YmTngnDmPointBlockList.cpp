#include "pch.h"
#include "YmTngnDmPointBlockList.h"
#include "YmTngnDraw.h"
#include "YmBase/YmBinaryFormatter.h"

using namespace std;
using namespace DirectX;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmTngnDmPointBlockList::YmTngnDmPointBlockList()
{
}

YmTngnDmPointBlockList::~YmTngnDmPointBlockList()
{
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmPointBlockList::SetScannerPosition(const YmVector3d& scannerPos)
{
	for (auto& inst : m_instanceList) {
		inst.pPointBlock->SetScannerPosition(scannerPos);
	}
}

bool YmTngnDmPointBlockList::IsUseScannerPosition() const
{
	for (auto& inst : m_instanceList) {
		if (inst.pPointBlock->IsUseScannerPosition()) {
			return true;
		}
	}
	return false;
}

void YmTngnDmPointBlockList::SetUseScannerPosition(bool isUse)
{
	for (auto& inst : m_instanceList) {
		inst.pPointBlock->SetUseScannerPosition(isUse);
	}
}

void YmTngnDmPointBlockList::AddInstance(InstanceData instance)
{
	m_instanceList.push_back(move(instance));
}

void YmTngnDmPointBlockList::AddInstances(const YmTngnDmPointBlockList& sourceInstances)
{
	YM_IS_TRUE(this != &sourceInstances);
	for (auto& instance : sourceInstances.GetInstanceList()) {
		m_instanceList.push_back(instance);
	}
}

////////////////////////////////////////////////////////////////////////////////

bool YmTngnDmPointBlockList::OnSetPickEnabled(bool bEnable)
{
	return bEnable;
}

/// <summary>
/// 
/// </summary>
/// <param name="modelToViewMatrix">modelToViewMatrix is pre-multiplied matrix by model coordinates.</param>
/// <param name="aabb"></param>
/// <param name="distanceLBIn"></param>
/// <returns></returns>
static double CalcPointListEnumerationPrecision(
	const XMFLOAT4X4& modelToViewMatrix, const YmAabBox3d& aabb, double distanceLBIn
)
{
	const double distanceLB = max(0, distanceLBIn);
	double minDistance = DBL_MAX;
	for (int i = 0; i < 8; ++i) {
		YmVector3d pnt;
		pnt[0] = ((i & 0x01) == 0 ? aabb.GetMinPoint()[0] : aabb.GetMaxPoint()[0]);
		pnt[1] = ((i & 0x02) == 0 ? aabb.GetMinPoint()[1] : aabb.GetMaxPoint()[1]);
		pnt[2] = ((i & 0x04) == 0 ? aabb.GetMinPoint()[2] : aabb.GetMaxPoint()[2]);
		double value = modelToViewMatrix.m[3][2];
		for (int j = 0; j < 3; ++j) {
			value += modelToViewMatrix.m[j][2] * pnt[j];
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
	const int64_t maxDrawnPointCountPerFrame = m_maxDrawnPointCountPerFrame;

	if (!pDraw->IsProgressiveViewFollowingFrame()) {
		UpdateDrawnInstances(pDraw);

		size_t nDrawnInst = m_drawnInstanceIndices.size();
		int64_t maxPointPerInst = maxDrawnPointCountPerFrame;
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

		uint64_t nextPickId = uint64_t(1) << 32;
		double persNearZ = pDraw->GetPerspectiveViewNearZ();
		for (auto iBlock : m_drawnInstanceIndices) {
			const InstanceData& instance = m_instanceList[iBlock];
			pDraw->SetModelMatrix(instance.localToGlobalMatrix);
			XMFLOAT4X4 modelToViewMatrix;
			XMStoreFloat4x4(&modelToViewMatrix, pDraw->GetModelToViewMatrix());
			double precision = CalcPointListEnumerationPrecision(modelToViewMatrix, instance.aabb, persNearZ);
			instance.pPointBlock->SetDrawingPrecision(precision);
			instance.pPointBlock->SetMaxPointCountDrawnPerFrame(maxPointPerInst);
			if (IsPickEnabled()) {
				instance.pPointBlock->SetPointPickTargetIdFirst(nextPickId);
				uint64_t pickIdEnd = nextPickId + instance.pPointBlock->GetPointCount();
				nextPickId = ((pickIdEnd >> 32) + 1 ) << 32;
				instance.pPointBlock->SetPickEnabled(true);
			}
			else {
				instance.pPointBlock->SetPickEnabled(false);
			}
			instance.pPointBlock->PrepareFirstDraw(pDraw);
		}
	}
	bool isProgressiveMode = pDraw->IsProgressiveViewMode();

	if (isProgressiveMode) {
		DrawInstancesInProgressiveMode(pDraw, maxDrawnPointCountPerFrame);
	}
	else {
		for (auto iBlock : m_drawnInstanceIndices) {
			const InstanceData& instance = m_instanceList[iBlock];
			pDraw->SetModelMatrix(instance.localToGlobalMatrix);
			instance.pPointBlock->Draw(pDraw);
		}
	}

	pDraw->ClearModelMatrix();
}

std::vector<YmTngnPointListVertex> YmTngnDmPointBlockList::OnFindPickedPoints(YmTngnPickTargetId id)
{
	std::vector<YmTngnPointListVertex> points;
	for (auto iInstance : m_drawnInstanceIndices) {
		const InstanceData& instance = m_instanceList[iInstance];
		YmTngnPointListVertex pointData;
		if (instance.pPointBlock->FindPointByPickTargetId(id, &pointData)) {
			points.push_back(pointData);
		}
	}
	return points;
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

	size_t nBlock = m_instanceList.size();
	using Distance = tuple<double, double>;
	multimap<Distance, size_t> distanceToBlock;
	for (size_t iBlock = 0; iBlock < nBlock; ++iBlock) {
		auto& block = m_instanceList[iBlock];
		pDraw->SetModelMatrix(block.localToGlobalMatrix);

		XMMATRIX modelViewMatrix = pDraw->GetModelToViewMatrix();
		XMMATRIX modelProjMatrix = pDraw->GetModelToProjectionMatrix();

		double scannerDistance = m_scannerDistanceForPointBlockWithoutScannerPos;
		if (block.pPointBlock->IsUseScannerPosition()) {
			XMFLOAT3 scannerPos = YmVectorUtil::StaticCast<XMFLOAT3>(block.pPointBlock->GetScannerPosition());
			XMVECTOR scannerPosInView = XMVector3Transform(XMLoadFloat3(&scannerPos), modelViewMatrix);
			scannerDistance = sqrt(XMVectorGetX(XMVector3Dot(scannerPosInView, scannerPosInView)));
		}
		double boxDistance = 0;
		bool isVisible = CalcBoxDistanceInProjection(modelProjMatrix, block.aabb, &boxDistance);
		if (!isVisible) {
			continue;
		}
		distanceToBlock.emplace(Distance(scannerDistance, boxDistance), iBlock);
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
