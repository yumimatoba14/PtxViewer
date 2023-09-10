//#define RGBA_TYPE float4

cbuffer ShaderParam : register(b0)
{
	matrix viewMatrix;
	matrix projectionMatrix;
	float pointSizeX;
	float pointSizeY;
	float pixelSizeX;
	float pixelSizeY;
	float3 scannerPosition;
	float scannerDistanceUpperBound;
	float scannerDistanceDepthOffset;
	int isUseScannerPosition;
};

struct VS_INPUT
{
	float3 Pos : POSITION;
	RGBA_TYPE Col : COLOR;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	RGBA_TYPE Col : COLOR;
};

float calcCameraDistanceOffset(float3 scannerPos, float4 posViewCoord)
{
	// Right hand coordinate system is assumed.
	if (0 <= posViewCoord[2]) {
		return 0;
	}
	float4 scannerPosViewCoord = float4(scannerPos, 1);
	scannerPosViewCoord = mul(scannerPosViewCoord, viewMatrix);
	scannerPosViewCoord[3] = 0;
	float dist = sqrt(mul(scannerPosViewCoord, scannerPosViewCoord));
	float distUpperBound = scannerDistanceUpperBound;
	if (distUpperBound < dist) {
		dist = distUpperBound;
	}
	return dist / distUpperBound * scannerDistanceDepthOffset;
}

PS_INPUT vsMain(VS_INPUT pos)
{
	PS_INPUT o = (PS_INPUT)0;
	float4 coord = float4(pos.Pos, 1);
	if (isUseScannerPosition) {
		float4 posViewCoord = mul(coord, viewMatrix);
		o.Pos = mul(posViewCoord, projectionMatrix);
		float zOffset = calcCameraDistanceOffset(scannerPosition, posViewCoord);
		// w をかけると奥の方の点が想定より早く消えてしまう。
		// w をかける方がオフセット値を線形にできるが、同次座標系のためかオフセット量が大きくなり制御が難しそう。
		// w をかけない方がオフセット量がz値に反比例するイメージになり、害が少ない。(これでも消えるタイミングは早いが。)
		//zOffset *= o.Pos[3];
		o.Pos[2] += zOffset;
	}
	else {
		coord = mul(coord, viewMatrix);
		o.Pos = mul(coord, projectionMatrix);
	}
	o.Col = pos.Col;
	return o;
}

float decideHalfPointSize(float givenPointSize, float pixelSize, float w)
{
	if (givenPointSize < 0) {
		return -0.5f * givenPointSize * w;	// givenPointSize has included pixelSize.
	}
	// draw as 1 pixel at minimum.
	return 0.5f * max(givenPointSize, pixelSize * w);
}

[maxvertexcount(4)]   // ジオメトリシェーダーで出力する最大頂点数
// ジオメトリシェーダー
void gsMain(point PS_INPUT inPoint[1],                       // ポイント プリミティブの入力情報
	inout TriangleStream<PS_INPUT> triStream  // トライアングル プリミティブの出力ストリーム
)
{
	PS_INPUT outPoint;
	const float halfSizeX = decideHalfPointSize(pointSizeX, pixelSizeX, inPoint[0].Pos[3]);
	const float halfSizeY = decideHalfPointSize(pointSizeY, pixelSizeY, inPoint[0].Pos[3]);

	outPoint.Pos = float4(inPoint[0].Pos[0] + halfSizeX, inPoint[0].Pos[1] + halfSizeY, inPoint[0].Pos[2], inPoint[0].Pos[3]);
	outPoint.Col = inPoint[0].Col;
	triStream.Append(outPoint);

	outPoint.Pos = float4(inPoint[0].Pos[0] - halfSizeX, inPoint[0].Pos[1] + halfSizeY, inPoint[0].Pos[2], inPoint[0].Pos[3]);
	outPoint.Col = inPoint[0].Col;
	triStream.Append(outPoint);

	outPoint.Pos = float4(inPoint[0].Pos[0] + halfSizeX, inPoint[0].Pos[1] - halfSizeY, inPoint[0].Pos[2], inPoint[0].Pos[3]);
	outPoint.Col = inPoint[0].Col;
	triStream.Append(outPoint);

	outPoint.Pos = float4(inPoint[0].Pos[0] - halfSizeX, inPoint[0].Pos[1] - halfSizeY, inPoint[0].Pos[2], inPoint[0].Pos[3]);
	outPoint.Col = inPoint[0].Col;
	triStream.Append(outPoint);

	triStream.RestartStrip();
}

RGBA_TYPE psMain(PS_INPUT input) : SV_TARGET
{
	return input.Col;
}
