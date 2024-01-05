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
	int isUseLight;		// For Triangle list.
	float materialAmbientCoef;
	float3 lightToObjectDir;
	float materialDiffuseCoef;
	float3 lightDiffuseRgb;
	float3 lightSpecularRgb;
	float lightSpecularShininess;
};
