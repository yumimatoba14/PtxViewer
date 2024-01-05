#include "pch.h"
#include "YmObjToIndexedTriangleListConverter.h"
#include "YmTngnIndexedTriangleList.h"
#include "YmBase/YmFilePath.h"
#include <algorithm>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

namespace {
	static int ToInt(double v) { return static_cast<int>(floor(v + 0.5)); }
	static int ToInt255(double v) { return static_cast<int>(floor(v * 255 + 0.5)); }
	static YmRgba4b ToRgb(const float aRgb[3])
	{
		return YmRgba4b(ToInt255(aRgb[0]), ToInt255(aRgb[1]), ToInt255(aRgb[2]));
	}

	static YmRgba4b ConvertMaterialToRgba(const tinyobj::material_t& material)
	{
		YmRgba4b rgb = YmObjToIndexedTriangleListConverter::ConvertMaterialColorToRgb(
			ToRgb(material.ambient), ToRgb(material.diffuse), ToRgb(material.specular)
		);
		return YmRgba4b(rgb.GetR(), rgb.GetG(), rgb.GetB(), ToInt255(material.dissolve));
	}

	struct LoaderContext {
		const YmObjToIndexedTriangleListConverter* pConverter;
		double zeroTol = 1e-6;
		tinyobj::attrib_t attrib;
		vector<tinyobj::material_t> materials;

		explicit LoaderContext(const YmObjToIndexedTriangleListConverter* pConverter) : pConverter(pConverter) {}

		YmVector3f GetVertexCoord(int index) const
		{
			size_t i = index;
			YM_ASSERT(0 <= index && 3 * i < attrib.vertices.size());
			return YmVectorUtil::Make<float>(
				attrib.vertices[3 * i + 0],
				attrib.vertices[3 * i + 1],
				attrib.vertices[3 * i + 2]);
		}

		// catmark_torus_creases0.obj has normal indices which don't exist.
		bool HasVertexNormal(int index) const
		{
			size_t i = index;
			return 0 <= index && i < attrib.normals.size();
		}
		YmVector3f GetVertexNormal(int index) const
		{
			size_t i = index;
			YM_ASSERT(HasVertexNormal(index));
			YmVector3f normal = YmVectorUtil::Make<float>(
				attrib.normals[3 * i + 0],
				attrib.normals[3 * i + 1],
				attrib.normals[3 * i + 2]);
			return YmVectorUtil::Normalize(zeroTol, normal);
		}

		YmRgba4b GetMatrialColor(int materialId) const
		{
			if (materialId < 0 || materials.size() <= materialId) {
				return pConverter->GetDefaultMaterialColor();
			}
			return ConvertMaterialToRgba(materials[materialId]);
		}
	};

	using MaterialType = int;
	using TriangleType = tuple<MaterialType, size_t>;
	using VertexType = tuple<int, int>;	//<vertex_index, normal_index>
}

////////////////////////////////////////////////////////////////////////////////

YmObjToIndexedTriangleListConverter::YmObjToIndexedTriangleListConverter()
{
}

YmObjToIndexedTriangleListConverter::~YmObjToIndexedTriangleListConverter()
{
}

////////////////////////////////////////////////////////////////////////////////

static void VerifyShapes(const vector<tinyobj::shape_t>& shapes)
{
	for (const tinyobj::shape_t& shape : shapes) {
		YM_IS_TRUE(shape.mesh.material_ids.size() == shape.mesh.num_face_vertices.size());
		size_t numMeshVertex = 0;
		for (size_t numVertex : shape.mesh.num_face_vertices) {
			YM_IS_TRUE(numVertex == 3);	// It must be triangulated.
			numMeshVertex += numVertex;
		}

		YM_IS_TRUE(shape.mesh.indices.size() == numMeshVertex);
	}
}

static vector<TriangleType> SortTrianglesByMaterial(const tinyobj::mesh_t& mesh)
{
	YM_IS_TRUE(mesh.material_ids.size() == mesh.num_face_vertices.size());
	vector<TriangleType> meshFaces;
	size_t nFace = mesh.material_ids.size();
	for (size_t iFace = 0; iFace < nFace; ++iFace) {
		meshFaces.emplace_back(mesh.material_ids[iFace], iFace);
	}
	sort(meshFaces.begin(), meshFaces.end());
	return meshFaces;
}

namespace {
	template<class ITERATOR, class INTEGER>
	ITERATOR AdvanceIterator(ITERATOR it, INTEGER offset) { advance(it, offset); return it; }

	template<class IndexIterator>
	YmVector3f CalcTriangleNormal(const LoaderContext* pLoader, IndexIterator itIndexBegin)
	{
		YmVector3f aCoord[3];
		for(int i = 0; i < 3; ++i) {
			auto itIndex = AdvanceIterator(itIndexBegin, i);
			aCoord[i] = pLoader->GetVertexCoord(itIndex->vertex_index);
		}
		YmVector3f aDir[2] = {
			aCoord[1] - aCoord[0], aCoord[2] - aCoord[0]
		};
		const double zeroTol = pLoader->zeroTol;
		// Ignore errors and just returns zero vector.
		YmVectorUtil::TryNormalize(zeroTol, aDir[0], aDir + 0);
		YmVectorUtil::TryNormalize(zeroTol, aDir[1], aDir + 1);
		YmVector3f normal = YmVectorUtil::OuterProduct(aDir[0], aDir[1]);
		if (YmVectorUtil::TryNormalize(zeroTol, normal, &normal)) {
			return normal;
		}
		return YmVector3f::MakeZero();
	}
}

static void ConvertMesh(LoaderContext* pLoader, const tinyobj::mesh_t& mesh, vector<YmTngnIndexedTriangleListPtr>* pResult)
{
	vector<TriangleType> triangles = SortTrianglesByMaterial(mesh);
	if (triangles.empty()) {
		return;
	}
	YmTngnIndexedTriangleListPtr pTriangles;
	map<VertexType, YmTngnIndexedTriangleList::VertexIndex> vertexIndexMap;

	size_t nTri = triangles.size();
	for (size_t iTri = 0; iTri < nTri; ++iTri) {
		int materialId = get<0>(triangles[iTri]);			
		if (pTriangles == nullptr) {
			pTriangles = make_shared<YmTngnIndexedTriangleList>();
			pTriangles->SetColor(pLoader->GetMatrialColor(materialId));
			pResult->push_back(pTriangles);
			vertexIndexMap.clear();
		}

		auto itIndexBegin = AdvanceIterator(mesh.indices.begin(), 3 * iTri);
		YmVector3f defaultNormalDir = CalcTriangleNormal(pLoader, itIndexBegin);
		auto itIndexEnd = AdvanceIterator(itIndexBegin, 3);
		auto resultTriangle = YmTngnIndexedTriangleList::TriangleType::MakeZero();
		for (int i = 0; i < 3; ++i) {
			auto itIndex = AdvanceIterator(itIndexBegin, i);

			VertexType vertexIndex = make_tuple(itIndex->vertex_index, itIndex->normal_index);
			auto itFoundVtx = vertexIndexMap.find(vertexIndex);
			if (itFoundVtx == vertexIndexMap.end()) {
				YM_IS_TRUE(0 <= itIndex->vertex_index);
				YmTngnIndexedTriangleList::VertexType vertex;
				vertex.position = pLoader->GetVertexCoord(itIndex->vertex_index);

				bool hasNormal = false;
				if (pLoader->HasVertexNormal(itIndex->normal_index)) {
					vertex.normalDir = pLoader->GetVertexNormal(itIndex->normal_index);
					hasNormal = true;
				}
				else {
					vertex.normalDir = defaultNormalDir;
				}

				resultTriangle[i] = pTriangles->AddVertex(vertex);
				if (hasNormal) {
					vertexIndexMap.insert(make_pair(vertexIndex, resultTriangle[i]));
				}
			}
			else {
				resultTriangle[i] = itFoundVtx->second;
			}
		}

		pTriangles->AddTriangle(resultTriangle);

		if (iTri + 1 == nTri) {
			pTriangles.reset();
		}
		else {
			int nextMatrialId = get<0>(triangles[iTri + 1]);
			if (materialId != nextMatrialId) {
				pTriangles.reset();
			}
		}
	}
}

std::vector<YmTngnIndexedTriangleListPtr> YmObjToIndexedTriangleListConverter::Convert(const char* pFilePath)
{
	YmFilePath objFilePath(pFilePath);
	string materialBaseDir = objFilePath.GetParentDirectory().ToString();
	LoaderContext context(this);
	std::vector<tinyobj::shape_t> shapes;
	std::string warn;
	std::string err;
	bool ret = tinyobj::LoadObj(&context.attrib, &shapes, &context.materials, &warn, &err,
		objFilePath.ToString().c_str(), materialBaseDir.c_str());
	YM_IS_TRUE(ret);

#if _DEBUG
	VerifyShapes(shapes);
#endif

	vector<YmTngnIndexedTriangleListPtr> resultTriangles;
	for (tinyobj::shape_t& shape : shapes) {
		ConvertMesh(&context, shape.mesh, &resultTriangles);
	}

	return resultTriangles;
}

////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Calculate mixture RGB value.
/// It is assumed that black is meaningless and it is exlucded from mixture.
/// Returns black if all values are black.
/// </summary>
/// <param name="ambient"></param>
/// <param name="diffuse"></param>
/// <param name="specular"></param>
/// <returns>Calculated value. Returned value's alpha must be 255.</returns>
YmRgba4b YmObjToIndexedTriangleListConverter::ConvertMaterialColorToRgb(
	const YmRgba4b& ambient, const YmRgba4b& diffuse, const YmRgba4b& specular
)
{
	YmRgba4b aInputRgb[3] = { ambient, diffuse, specular };
	for (int i = 0; i < 3; ++i) {
		aInputRgb[i] = aInputRgb[i].RemoveAlpha();
	}
	const YmRgba4b* const aRgb = aInputRgb;
	const YmRgba4b black(0, 0, 0);
	int numBlack = 0;
	int notBlackIndex = -1;
	int blackIndex = -1;
	for (int i = 0; i < 3; ++i) {
		if (aRgb[i] == black) {
			++numBlack;
			blackIndex = i;
		}
		else {
			notBlackIndex = i;
		}
	}
	if (numBlack == 3) {
		return black;
	}
	YM_ASSERT(0 <= notBlackIndex && notBlackIndex < 3);
	if (numBlack == 2) {
		return aRgb[notBlackIndex];
	}

	int aRgbSum[3] = { 0, 0, 0 };
	for (int i = 0; i < 3; ++i) {
		if (i == blackIndex) {
			continue;
		}
		aRgbSum[0] += aRgb[i].GetR();
		aRgbSum[1] += aRgb[i].GetG();
		aRgbSum[2] += aRgb[i].GetB();
	}
	double denominator = 3 - numBlack;
	return YmRgba4b(ToInt(aRgbSum[0] / denominator), ToInt(aRgbSum[1] / denominator), ToInt(aRgbSum[2] / denominator));
}

////////////////////////////////////////////////////////////////////////////////
