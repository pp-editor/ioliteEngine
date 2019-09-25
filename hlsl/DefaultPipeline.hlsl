cbuffer CBPerFrame       : register(b0) {
	matrix View;
	matrix Proj;
}
cbuffer CBUnitFrame      : register(b1) {
	matrix World;
}
cbuffer CBMaterial       : register(b2) {
	float4 Mate_ambient;
	float4 Mate_diffuse;
	float4 Mate_specular;
}
cbuffer CBPerLight       : register(b3) {
	matrix LightView;
	matrix LightProj;
}
cbuffer CBLightProperty  : register(b4) {
	float4 EyePosition;
}
cbuffer CBAmbientLight   : register(b5) {
	float4 ALight_color;
}
cbuffer CBDirectionLight : register(b6) {
	float4 DLight_position;
	float4 DLight_direction;
	float4 DLight_color;
}
cbuffer CBPointLight     : register(b7) {
	float4 PLight_position;
	float4 PLight_diffuse;
	float4 PLight_specular;
	float4 PLight_shininess;
}

Texture2D              Texture       : register(t0);
TextureCube            CubeTex       : register(t1);
Texture2D              DepthMap      : register(t2);
SamplerState           Sampler       : register(s0);
SamplerComparisonState SamplerDepth  : register(s1);

struct VSInput {
	float3 Position : POSITION;
	float3 Normal   : NORMAL;
	float4 Color    : COLOR;
	float2 TexCoord : TEXCOORD;
};

struct GSPSInput {
	float4 Position     : SV_POSITION;
	float4 Position0    : POSITION0;
	float3 Normal       : NORMAL;
	float4 Color        : COLOR;
	float2 TexCoord     : TEXCOORD;
	float3 CubeCoord    : TEXCOORD1;
	float4 LightViewPos : TEXCOORD2;
	float3 LightPos     : TEXCOORD3;
};

GSPSInput vsMain(VSInput input) {
	GSPSInput output = (GSPSInput)0;
	//! calc object
	float4 WorldPos  = mul(World, float4(input.Position, 1.0f));
	output.Position  = mul(View,  WorldPos);
	output.Position  = mul(Proj,  output.Position);
	float3 normal    = normalize(mul(float4(input.Normal, 1.0f), World).xyz);

	//! calc light
	output.LightViewPos = mul(World, float4(input.Position, 1.0f));
	output.LightViewPos = mul(LightView, output.LightViewPos);
	output.LightViewPos = mul(LightProj, output.LightViewPos);

	//! set values
	output.Position0    = WorldPos;
	output.Normal       = normal;
	output.Color        = input.Color;
	output.TexCoord     = input.TexCoord;
	output.CubeCoord    = input.Position;
	output.LightPos     = normalize(DLight_position.xyz - WorldPos.xyz);
	return output;
}

[maxvertexcount(3)]
void gsMain(triangle GSPSInput input[3], inout TriangleStream<GSPSInput> stream) {
	for (int i = 0; i < 3; i++) {
		GSPSInput output    = (GSPSInput)0;
		output.Position     = input[i].Position;
		output.Position0    = input[i].Position0;
		output.Normal       = input[i].Normal;
		output.Color        = input[i].Color;
		output.TexCoord     = input[i].TexCoord;
		output.CubeCoord    = input[i].CubeCoord;
		output.LightViewPos = input[i].LightViewPos;
		output.LightPos     = input[i].LightPos;
		stream.Append(output);
	}
	stream.RestartStrip();
}

float4 shadowMapping(GSPSInput input) {
	float  bias  = 0.01f;
	float3 projCoord = input.LightViewPos.xyz / input.LightViewPos.w;
	projCoord.x = 0.5f + projCoord.x * 0.5f;
	projCoord.y = 0.5f - projCoord.y * 0.5f;
	float  threshold = DepthMap.SampleCmpLevelZero(SamplerDepth, projCoord.xy, projCoord.z - bias).r;
	float4 color     = lerp(ALight_color, DLight_color, threshold);
	return color;
}

float4 blinnPhong(GSPSInput input, float3 ambient) {
	float3 n = input.Normal;
	float3 v = normalize(EyePosition.xyz - input.Position0.xyz);
	float3 l = PLight_position.xyz - input.Position0.xyz;
	float  d = length(l);
	l = normalize(l);
	float3 r = 2.0f * n * dot(n, l) - 1;
	float3 a = saturate(1.0f / (PLight_shininess.x + PLight_shininess.y * d + PLight_shininess.z * d * d));

	float3 iA = Mate_ambient.xyz * ambient;
	float3 iD = saturate(dot(l, n)) * Mate_diffuse.xyz * PLight_diffuse.xyz * a;
	float3 iS = pow(saturate(dot(r, v)), Mate_specular.w) * Mate_specular.xyz * PLight_specular.xyz * a;
	return float4(saturate(iA+iD+iS), 1.0f);
}

float4 lightColorBlend(GSPSInput input) {
	return blinnPhong(input, shadowMapping(input).xyz);
}

void psMain(in GSPSInput input, out float4 output : SV_TARGET0) {
	output = Texture.Sample(Sampler, input.TexCoord) * input.Color * lightColorBlend(input);
	clip(output.a - 0.0001f); 
}

void psMainNoTexture(in GSPSInput input, out float4 output : SV_TARGET0) {
	output =  input.Color * lightColorBlend(input);
	clip(output.a - 0.0001f);
}

void psMainCube(in GSPSInput input, out float4 output : SV_TARGET0) {
	output = CubeTex.Sample(Sampler, input.CubeCoord) * input.Color * lightColorBlend(input);
	clip(output.a - 0.0001f);
}

void psMainCubeMap(in GSPSInput input, out float4 output : SV_TARGET0) {
	float3 E = normalize(input.Position0.xyz - EyePosition.xyz);
	float3 R = reflect(E, input.Normal);
	output = CubeTex.Sample(Sampler, R) * input.Color * blinnPhong(input, ALight_color.xyz);
}

GSPSInput vsMainDepth(VSInput input) {
	GSPSInput output = (GSPSInput)0;
	float4 WorldPos  = mul(World, float4(input.Position, 1.0f));
	float4 ViewPos   = mul(LightView,  WorldPos);
	float4 ProjPos   = mul(LightProj,  ViewPos);
	output.Position  = ProjPos;
	return output;
}