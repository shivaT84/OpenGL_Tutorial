#version 410

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inWorldPosition;
layout(location=3) in mat3 inTBN;//接空間行列.

out vec4 fragColor;
// 0番目にカラーテクスチャ、1番めにノーマルマップテクスチャが渡されることを想定.
uniform sampler2D colorSampler[2];

// ライトデータ(点光源).
struct PointLight {
	vec4 position; //座標(ワールド座標系).
	vec4 color; // 明るさ.
}; 
 
const int maxLightCount = 4; // ライトの数. 
 
// ライティングパラメータ.
layout(std140) uniform LightData {
	vec4 ambientColor; // 環境光.
	PointLight light[maxLightCount]; // ライトのリスト.
} lightData;


void main() {
	// ノーマルマップテクスチャを読み取り、逆接ベクトル空間行列を掛けて法線をワールド座標系に変換.
	vec3 normal =texture(colorSampler[1], inTexCoord).xyz * 2.0 - 1.0;
	normal = inTBN * normal;

	vec3 lightColor = lightData.ambientColor.rgb;
	for (int i = 0; i < maxLightCount; ++i) {
		vec3 lightVector = lightData.light[i].position.xyz - inWorldPosition; 
		float lightPower = 1.0 / dot(lightVector, lightVector);
		float cosTheta = clamp(dot(normal, normalize(lightVector)), 0, 1);
		lightColor += lightData.light[i].color.rgb * cosTheta * lightPower;
	}
	fragColor = inColor * texture(colorSampler[0], inTexCoord);//カラーテクスチャの取得.
	fragColor.rgb *= lightColor;
}