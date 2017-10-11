#version 410

layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexCoord;
layout(location=3) in vec3 vNormal;
layout(location=4) in vec4 vTangent;//�ڃx�N�g������.

layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexCoord;
layout(location=2) out vec3 outWorldPosition;
// �ڃx�N�g��(Tangent)�A�]�@���x�N�g��(Binormal)�A�@���x�N�g��(Normal).
layout(location=3) out mat3 outTBN;//�ڋ�ԍs��.

/**
* ���_�V�F�[�_�̃p�����[�^.
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

	// �ڋ�Ԃ��\������3�̃x�N�g���ɉ�]�s����|���ă��[���h���W�n�ɕϊ�.
	mat3 matNormal = mat3(vertexData.matNormal);
	vec3 t = matNormal * vTangent.xyz;
	vec3 n = matNormal * vNormal;
	vec3 b = normalize(cross(n, t)) * vTangent.w;
	// 3x3�s��ɓ]�u���ċt�s����쐬.
	outTBN = mat3(t, b, n);

	gl_Position = vertexData.matMVP * vec4(vPosition, 1.0);
}