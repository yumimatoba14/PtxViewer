#pragma once

#include "YmBaseFwd.h"
#include "YmVector.h"

namespace Ymcpp {

/// <summary>
/// This class represents an orthgonal transformation of 3D coordinates.
/// This class usually converts a point from its local coordinte system into the global coordinate system.
/// Make an inversed object to execute an conversion in the reversed direction.
/// </summary>
/// <typeparam name="COORD">floating point type for coordinates.</typeparam>
template<class COORD>
class YmOrtho3dXform
{
public:
	using CoordType = COORD;
	using VectorType = YmVectorN<3, CoordType>;
private:
	static VectorType MakeVector(CoordType x, CoordType y, CoordType z) { return YmVectorUtil::Make<CoordType>(x, y, z); }

	YmOrtho3dXform(
		const VectorType& org, const VectorType& dirX, const VectorType& dirY, const VectorType& dirZ
	)
	{
		m_localOrigin = org;
		m_aLocalDir[0] = dirX;
		m_aLocalDir[1] = dirY;
		m_aLocalDir[2] = dirZ;
	}

public:
	/// <summary>
	/// Make a transformation corresponding to unit matrix.
	/// </summary>
	YmOrtho3dXform()
		: YmOrtho3dXform(
			VectorType::MakeZero(),
			MakeVector(1, 0, 0), MakeVector(0, 1, 0), MakeVector(0, 0, 1)
		)
	{
	}
#if 0
	virtual ~YmOrtho3dXform();

	YmOrtho3dXform(const YmOrtho3dXform& other) = delete;
	YmOrtho3dXform(YmOrtho3dXform&& other) noexcept = delete;

	YmOrtho3dXform& operator = (const YmOrtho3dXform& other) = delete;
	YmOrtho3dXform& operator = (YmOrtho3dXform&& other) noexcept = delete;
#endif

	/// <summary>
	/// Make a transformation to translate a point without rotation.
	/// </summary>
	/// <param name="org"></param>
	/// <returns></returns>
	static YmOrtho3dXform MakeTranslation(const VectorType& org)
	{
		return YmOrtho3dXform(
			org, MakeVector(1, 0, 0), MakeVector(0, 1, 0), MakeVector(0, 0, 1)
		);
	}

	/// <summary>
	/// Make a transformation to rotate point around Z axis.
	/// </summary>
	/// <param name="angleRad">rotation angle in degree.</param>
	/// <returns></returns>
	static YmOrtho3dXform MakeRotationAroundZ(CoordType angleRad)
	{
		CoordType c = cos(angleRad);
		CoordType s = sin(angleRad);
		return YmOrtho3dXform(
			VectorType::MakeZero(), MakeVector(c, s, 0), MakeVector(-s, c, 0), MakeVector(0, 0, 1)
		);
	}

	/// <summary>
	/// Make a general transformation with specifying local x and y directions.
	/// If dirX and dirY is not perpendicular, this function modify them to make
	/// orthogonal coordinate system.
	/// </summary>
	/// <param name="org"></param>
	/// <param name="dirX"></param>
	/// <param name="dirY"></param>
	/// <returns></returns>
	static YmOrtho3dXform MakeFromXy(const VectorType& org, const VectorType& dirX, const VectorType& dirY)
	{
		const double zeroTol = 1e-12;
		VectorType dirXMod = YmVectorUtil::Normalize(zeroTol, dirX);
		VectorType dirZ = YmVectorUtil::Normalize(zeroTol, YmVectorUtil::OuterProduct(dirXMod, dirY));
		VectorType dirYMod = YmVectorUtil::OuterProduct(dirZ, dirXMod);
		return YmOrtho3dXform(org, dirXMod, dirYMod, dirZ);
	}

public:
	/// <summary>
	/// Get the origin of the local coordinate system in the global coordinate system.
	/// </summary>
	/// <returns></returns>
	VectorType GetLocalOrigin() const { return m_localOrigin; }
	VectorType GetLocalXAxisDir() const { return m_aLocalDir[0]; }
	VectorType GetLocalYAxisDir() const { return m_aLocalDir[1]; }
	VectorType GetLocalZAxisDir() const { return m_aLocalDir[2]; }

	/// <summary>
	/// Return 4x4 matrix in row major of this transformation.
	/// The local coordinate shall be producted from right side of the matrix.
	/// The matrix can be written like this;
	/// [ m[0],  m[1],  m[2],  m[3]  ]
	/// [ m[4],  m[5],  m[6],  m[7]  ]
	/// [ m[8],  m[9],  m[10], m[11] ]
	/// [ m[12], m[13], m[14], m[15] ]
	/// </summary>
	/// <param name="m">Array to store a row major matrix.</param>
	void GetRowMajorPostMultipliedMatrix(CoordType m[16]) const
	{
		YM_IS_TRUE(m != nullptr);
		for (int i = 0; i < 3; ++i) {
			m[i * 4 + 0] = m_aLocalDir[0][i];
			m[i * 4 + 1] = m_aLocalDir[1][i];
			m[i * 4 + 2] = m_aLocalDir[2][i];
			m[i * 4 + 3] = m_localOrigin[i];
		}
		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
		m[15] = 1;
	}

	/// <summary>
	/// Return 4x4 matrix in row major of this transformation.
	/// The local coordinate shall be producted from left side of the matrix.
	/// The matrix can be written like this;
	/// [ m[0],  m[1],  m[2],  m[3]  ]
	/// [ m[4],  m[5],  m[6],  m[7]  ]
	/// [ m[8],  m[9],  m[10], m[11] ]
	/// [ m[12], m[13], m[14], m[15] ]
	/// </summary>
	/// <param name="m">Array to store a row major matrix.</param>
	void GetRowMajorPreMultipliedMatrix(CoordType m[16]) const
	{
		YM_IS_TRUE(m != nullptr);
		YmVectorUtil::CopyToArray(m_aLocalDir[0], 3, m + 0);
		YmVectorUtil::CopyToArray(m_aLocalDir[1], 3, m + 4);
		YmVectorUtil::CopyToArray(m_aLocalDir[2], 3, m + 8);
		YmVectorUtil::CopyToArray(m_localOrigin, 3, m + 12);
		m[3] = 0;
		m[7] = 0;
		m[11] = 0;
		m[15] = 1;
	}

	VectorType ConvertCoord(const VectorType& localPoint) const
	{
		VectorType globalPoint = m_localOrigin;
		for (int i = 0; i < 3; ++i) {
			globalPoint += localPoint[i] * m_aLocalDir[i];
		}
		return globalPoint;
	}

	VectorType ConvertDirection(const VectorType& localDir) const
	{
		VectorType globalDir = VectorType::MakeZero();
		for (int i = 0; i < 3; ++i) {
			globalDir += localDir[i] * m_aLocalDir[i];
		}
		return globalDir;
	}

	/// <summary>
	/// Return inverse transformation.
	/// </summary>
	/// <returns></returns>
	YmOrtho3dXform Inverse() const
	{
		// Y = R * X + O
		// ->
		// X = Rinv * Y - Rinv * O
		// , where Rinv is inverse matrix of R, O is local origin.
		using namespace YmVectorUtil;
		VectorType invOrg = -1 * MakeVector(
			InnerProduct(m_aLocalDir[0], m_localOrigin),
			InnerProduct(m_aLocalDir[1], m_localOrigin),
			InnerProduct(m_aLocalDir[2], m_localOrigin)
		);
		VectorType aInvDir[3];
		for (int i = 0; i < 3; ++i) {
			aInvDir[i] = MakeVector(m_aLocalDir[0][i], m_aLocalDir[1][i], m_aLocalDir[2][i]);
		}
		return YmOrtho3dXform(invOrg, aInvDir[0], aInvDir[1], aInvDir[2]);
	}

	/// <summary>
	/// Return a transformation of lhs(rhs(localPoint)),
	/// where xform(localPoint) corresponds to xform.ConvertCoord(localPoint).
	/// </summary>
	/// <param name="lhs"></param>
	/// <param name="rhs"></param>
	/// <returns></returns>
	friend inline YmOrtho3dXform operator * (const YmOrtho3dXform& lhs, const YmOrtho3dXform& rhs)
	{
		VectorType resultOrg = lhs.ConvertCoord(rhs.GetLocalOrigin());
		VectorType aResultDir[3];
		for (int i = 0; i < 3; ++i) {
			aResultDir[i] = VectorType::MakeZero();
			for (int j = 0; j < 3; ++j) {
				aResultDir[i] += lhs.m_aLocalDir[j] * rhs.m_aLocalDir[i][j];
			}
		}
		return YmOrtho3dXform(resultOrg, aResultDir[0], aResultDir[1], aResultDir[2]);
	}

private:
	VectorType m_localOrigin;
	VectorType m_aLocalDir[3];
};

}
