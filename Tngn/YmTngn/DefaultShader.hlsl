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
		// w ��������Ɖ��̕��̓_���z���葁�������Ă��܂��B
		// w ������������I�t�Z�b�g�l����`�ɂł��邪�A�������W�n�̂��߂��I�t�Z�b�g�ʂ��傫���Ȃ萧�䂪������B
		// w �������Ȃ������I�t�Z�b�g�ʂ�z�l�ɔ���Ⴗ��C���[�W�ɂȂ�A�Q�����Ȃ��B(����ł�������^�C�~���O�͑������B)
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

[maxvertexcount(4)]   // �W�I���g���V�F�[�_�[�ŏo�͂���ő咸�_��
// �W�I���g���V�F�[�_�[
void gsMain(point PS_INPUT inPoint[1],                       // �|�C���g �v���~�e�B�u�̓��͏��
	inout TriangleStream<PS_INPUT> triStream  // �g���C�A���O�� �v���~�e�B�u�̏o�̓X�g���[��
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
