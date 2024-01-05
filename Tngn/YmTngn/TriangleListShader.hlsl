#include "ShaderParam.hlsl"

struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 Normal: NORMAL;
	float4 Col : COLOR;
#if PICKABLE_MODE
	uint4 PickTargetId : PICK_ID;
#endif
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Col : COLOR;
	float3 Normal : TEXCOORD;
#if PICKABLE_MODE
	uint4 PickTargetId : PICK_ID;
#endif
};

struct PS_OUTPUT
{
	float4 Color : SV_TARGET0;
#if PICKABLE_MODE
	uint4 PickTargetId : SV_TARGET1;
#endif
};

#if PICKABLE_MODE
#define COPY_PICK_TARGET_ID(o, i) (o).PickTargetId = (i).PickTargetId
#else
#define COPY_PICK_TARGET_ID(o, i) (0)
#endif

PS_INPUT vsMain(VS_INPUT pos)
{
	PS_INPUT o = (PS_INPUT)0;
	float4 coord = float4(pos.Pos, 1);
	coord = mul(coord, viewMatrix);
	o.Pos = mul(coord, projectionMatrix);
	o.Col = pos.Col;
	float4 normDir = float4(pos.Normal, 0);
	normDir = mul(normDir, viewMatrix);
	o.Normal = float3(normDir[0], normDir[1], normDir[2]);
	COPY_PICK_TARGET_ID(o, pos);
	return o;
}

float4 CalculateColor(float4 ambientRgba, float3 coordNormal)
{
	// Currently it is assumed that both side of triangles are drawn. (Triangle directions are ignored.)
	float diffuseStrength = -1 * dot(coordNormal, lightToObjectDir);
	diffuseStrength = abs(diffuseStrength);
	float3 refectionDir = reflect(lightToObjectDir, coordNormal);
	float specularStrength = dot(coordNormal, refectionDir);
	specularStrength = abs(specularStrength);
	specularStrength = pow(specularStrength, lightSpecularShininess);
	float3 rgb = float3(ambientRgba[0], ambientRgba[1], ambientRgba[2]) * (materialAmbientCoef + diffuseStrength * materialDiffuseCoef)
		+ diffuseStrength * lightDiffuseRgb + specularStrength * lightSpecularRgb;

	rgb = min(max(rgb, float3(0, 0, 0)), float3(1, 1, 1));
	return float4(rgb[0], rgb[1], rgb[2], ambientRgba[3]);
}

PS_OUTPUT psMain(PS_INPUT input)
{
	PS_OUTPUT output;
	if (isUseLight) {
		output.Color = CalculateColor(input.Col, normalize(input.Normal));
	}
	else {
		output.Color = input.Col;
	}
	COPY_PICK_TARGET_ID(output, input);
	return output;
}
