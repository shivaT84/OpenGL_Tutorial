#version 410

layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexCoord;
layout(location=3) in vec3 vNormal;
layout(location=4) in vec4 vTangent;//接ベクトル入力.

layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexCoord;
layout(location=2) out vec3 outWorldPosition;
// 接ベクトル(Tangent)、従法線ベクトル(Binormal)、法線ベクトル(Normal).
layout(location=3) out mat3 outTBN;//接空間行列.

/**
* 頂点シェーダのパラメータ.
*/
layout(std140) uniform VertexData {
	mat4 matMVP;
	mat4 matModel;
	mat3x4 matNormal;
	vec4 color;
} vertexData;


void main() {

	outColor = vColor * vertexData.color;
	outTexCoord = vTexCoord;
	outWorldPosition = (vertexData.matModel * vec4(vPosition, 1.0)).xyz;

	// 接空間を構成する3つのベクトルに回転行列を掛けてワールド座標系に変換.
	mat3 matNormal = mat3(vertexData.matNormal);
	vec3 t = matNormal * vTangent.xyz;
	vec3 n = matNormal * vNormal;
	vec3 b = normalize(cross(n, t)) * vTangent.w;
	// 3x3行列に転置して逆行列を作成.
	outTBN = mat3(t, b, n);

	gl_Position = vertexData.matMVP * vec4(vPosition, 1.0);
}