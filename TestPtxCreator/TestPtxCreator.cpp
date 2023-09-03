// TestPtxCreator.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//
#include "pch.h"
#include <iostream>
#include <fstream>
#include "YmRaycastTargetList.h"
#include "YmRtRectangle.h"
#include "YmBase/YmOrtho3dXform.h"
#include <iomanip>

using namespace std;
using namespace Ymcpp;

void WriteDouble3(ostream& os, double x, double y, double z)
{
	os << x << " " << y << " " << z << endl;
}

void WriteDouble3(ostream& os, const YmVector3d& vec)
{
	WriteDouble3(os, vec[0], vec[1], vec[2]);
}

void WriteDouble4(ostream& os, double x, double y, double z, double w)
{
	os << x << " " << y << " " << z << " " << w << endl;
}

void WriteDouble4(ostream& os, const YmVector3d& vec, double w)
{
	WriteDouble4(os, vec[0], vec[1], vec[2], w);
}

void WritePtxFile(
	const char* ptxFilePath,
	YmRaycastTarget* pTarget, const YmVector3d& origin, const YmVector3d& dirN, const YmVector3d& dirX,
	double bottomAngleRad, double topAngleRad, int nLongiDiv, int nLatiDiv,
	int rgb
)
{
	YM_IS_TRUE(1 < nLongiDiv);
	YM_IS_TRUE(1 < nLatiDiv);
	YmOrtho3dXform<double> localToGlobal = YmOrtho3dXform<double>::MakeFromXy(
		origin, dirX, YmVectorUtil::OuterProduct(dirN, dirX)
	);
	YmOrtho3dXform<double> globalToLocal = localToGlobal.Inverse();

	vector<YmVector2d> longitudeXy;
	for (int iLongiDiv = 0; iLongiDiv < nLongiDiv; ++iLongiDiv) {
		double rad = 2 * M_PI / nLongiDiv * iLongiDiv;
		longitudeXy.push_back(YmVectorUtil::Make(cos(rad), sin(rad)));
	}

	vector<YmVector2d> lattitudeHz;
	for (int iLatiDiv = 0; iLatiDiv < nLatiDiv; ++iLatiDiv) {
		double rad = bottomAngleRad + (topAngleRad - bottomAngleRad) / (nLatiDiv - 1) * iLatiDiv;
		lattitudeHz.push_back(YmVectorUtil::Make(cos(rad), sin(rad)));
	}

	ofstream ofs(ptxFilePath, ios::out);
	if (!ofs.is_open()) {
		YM_THROW_ERROR("Failed to open ptx file.");
	}

	int orgPrec = (int)ofs.precision();
	ofs << setprecision(16);
	ofs << nLongiDiv << endl;
	ofs << nLatiDiv << endl;
	WriteDouble3(ofs, localToGlobal.GetLocalOrigin());
	WriteDouble3(ofs, localToGlobal.GetLocalXAxisDir());
	WriteDouble3(ofs, localToGlobal.GetLocalYAxisDir());
	WriteDouble3(ofs, localToGlobal.GetLocalZAxisDir());
	WriteDouble4(ofs, localToGlobal.GetLocalXAxisDir(), 0);
	WriteDouble4(ofs, localToGlobal.GetLocalYAxisDir(), 0);
	WriteDouble4(ofs, localToGlobal.GetLocalZAxisDir(), 0);
	WriteDouble4(ofs, localToGlobal.GetLocalOrigin(), 1);
	ofs << setprecision(orgPrec);

	for (int iLongiDiv = 0; iLongiDiv < nLongiDiv; ++iLongiDiv) {
		for (int iLatiDiv = 0; iLatiDiv < nLatiDiv; ++iLatiDiv) {
			YmVector3d localDir = YmVectorUtil::Make(
				lattitudeHz[iLatiDiv][0] * longitudeXy[iLongiDiv][0],
				lattitudeHz[iLatiDiv][0] * longitudeXy[iLongiDiv][1],
				lattitudeHz[iLatiDiv][1]
			);
			YmVector3d globalDir = localToGlobal.ConvertDirection(localDir);

			double rayParam = 0;
			YmVector3d globalFootPoint;
			bool isInt = pTarget->RaycastFrom(origin, globalDir, &rayParam, &globalFootPoint);
			YmVector3d footPoint;
			if (!isInt) {
				footPoint = YmVector3d::MakeZero();
			}
			else {
				footPoint = globalToLocal.ConvertCoord(globalFootPoint);
			}
			double intencity = 1;
			int r = 0xFF & rgb;
			int g = 0xFF & (rgb >> 8);
			int b = 0xFF & (rgb >> 16);
			ofs << footPoint[0] << " " << footPoint[1] << " " << footPoint[2] << " "
				<< intencity << " " << r << " " << g << " " << b << endl;
		}
	}
}

void WritePtxFile(
	const char* pPtxFilePath,
	YmRaycastTarget* pTarget, const YmVector3d& origin, const YmVector3d& dirN, const YmVector3d& dirX, int rgb
)
{
	WritePtxFile(pPtxFilePath, pTarget, origin, dirN, dirX,
		-0.4 * M_PI, 0.4 * M_PI, 100, 50, rgb);
}

int main()
{
	const YmVector3d dirX = YmVectorUtil::Make(1, 0, 0);
	const YmVector3d dirY = YmVectorUtil::Make(0, 1, 0);
	const YmVector3d dirZ = YmVectorUtil::Make(0, 0, 1);
	YmRaycastTargetPtr pFloorPlane = std::make_shared<YmRtRectangle>(
		YmVectorUtil::Make(0, 0, 0), dirX, dirY, -100, 100, -100, 100
	);

	string ptxFilePath = "test_file.ptx";
	//WritePtxFile(ptxFilePath.c_str(), pFloorPlane.get(), YmVectorUtil::Make<double>(-3, 0, 1.8), dirZ, dirX, 0xFFFFFF);

	auto pRtList = make_shared<YmRaycastTargetList>();
	pRtList->Add(pFloorPlane);
	pRtList->Add(make_shared<YmRtRectangle>(YmVectorUtil::Make(1, 0, 0), dirY, dirZ, 0, 1, 0, 1));
	pRtList->Add(make_shared<YmRtRectangle>(YmVectorUtil::Make(0, 0, 0), dirX, dirZ, 0, 1, 0, 1));
	pRtList->Add(make_shared<YmRtRectangle>(YmVectorUtil::Make(1, 1, 0), -1*dirX, dirZ, 0, 1, 0, 1));
	pRtList->Add(make_shared<YmRtRectangle>(YmVectorUtil::Make(0, 0, 0), dirY, dirZ, 0, 1, 0, 1));

	double bottomAngleRad = -0.4 * M_PI;
	double topAngleRad = 0.5 * M_PI;
	const int nLongiDiv = 2000;
	const int nLatiDiv = 1000;
#if 1
	ptxFilePath = "test1_-3_0_1.8.ptx";
	WritePtxFile(ptxFilePath.c_str(), pRtList.get(), YmVectorUtil::Make<double>(-3, 0, 1.8), dirZ, dirX,
		bottomAngleRad, topAngleRad, nLongiDiv, nLatiDiv, 0xFF8080);
	ptxFilePath = "test1_0_-3_1.8.ptx";
	WritePtxFile(ptxFilePath.c_str(), pRtList.get(), YmVectorUtil::Make<double>(0, -3, 1.8), dirZ, dirX,
		bottomAngleRad, topAngleRad, nLongiDiv, nLatiDiv, 0x80FF80);
	ptxFilePath = "test1_2_-2_1.ptx";
	WritePtxFile(ptxFilePath.c_str(), pRtList.get(), YmVectorUtil::Make<double>(2, -2, 1), dirZ, dirY,
		bottomAngleRad, topAngleRad, nLongiDiv, nLatiDiv, 0x80DDDD);
	ptxFilePath = "test1_3_0_1.8.ptx";
	WritePtxFile(ptxFilePath.c_str(), pRtList.get(), YmVectorUtil::Make<double>(3, 0, 1.8), dirZ, dirY,
		bottomAngleRad, topAngleRad, nLongiDiv, nLatiDiv, 0x8080FF);
	ptxFilePath = "test1_0_3_1.8.ptx";
	WritePtxFile(ptxFilePath.c_str(), pRtList.get(), YmVectorUtil::Make<double>(0, 3, 1.8), dirZ, dirY,
		bottomAngleRad, topAngleRad, nLongiDiv, nLatiDiv, 0xFF88FF);
#endif

	return 0;
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
