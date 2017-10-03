/**
* @file	Main.cpp
*/

#include "GLFWEW.h"
#include "Texture.h"
#include "Shader.h"
#include "OffscreenBuffer.h"
#include "UniformBuffer.h"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream> 
#include <vector>


// ���_�f�[�^�^�̒�`
/// 3D �x�N�^�[�^.
struct Vector3 {
	float x, y, z;
}; 
/// RGBA �J���[�^.
struct Color {
	float r, g, b, a;
}; 
/// ���_�f�[�^�^.
struct Vertex {
	Vector3 position; ///< ���W 
	Color color; ///< �F
	glm::vec2 texCoord; ///< �e�N�X�`�����W
};


/// ���_�f�[�^.
const Vertex vertices[] = {
	{ {-0.5f,-0.3f, 0.5f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
	{ { 0.3f,-0.3f, 0.5f },{ 0.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 0.3f, 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
	{ {-0.5f, 0.5f, 0.5f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },

	{ {-0.3f, 0.3f, 0.1f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
	{ {-0.3f,-0.5f, 0.1f },{ 0.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
	{ { 0.5f,-0.5f, 0.1f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 0.5f,-0.5f, 0.1f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, 0.3f, 0.1f },{ 1.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
	{ {-0.3f, 0.3f, 0.1f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },

	{ {-1.0f,-1.0f, 0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 1.0f,-1.0f, 0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
	{ { 1.0f, 1.0f, 0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
	{ {-1.0f, 1.0f, 0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
};

/// �C���f�b�N�X�f�[�^.
const GLuint indices[] = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 7, 8, 9,
	10, 11, 12, 12, 13, 10,
};

/// ���_�V�F�[�_�̃p�����[�^�^.
struct VertexData {
	glm::mat4 matMVP;
	glm::vec4 lightPosition;
	glm::vec4 lightColor;
	glm::vec4 ambientColor;
};

/**
* ���C�g�f�[�^(�_����).
*/
struct PointLight {
	glm::vec4 position; ///< ���W(���[���h���W�n).
	glm::vec4 color; ///< ���邳.
}; 

const int maxLightCount = 4; ///< ���C�g�̐�. 

/**
* ���C�e�B���O�p�����[�^.
*/
struct LightData {
	glm::vec4 ambientColor; ///< ����.
	PointLight light[maxLightCount]; ///< ���C�g�̃��X�g.
};

/** *
�|�X�g�G�t�F�N�g�f�[�^.
*/ struct PostEffectData {
	glm::mat4x4 matColor; ///< �F�ϊ��s��.
};

/**
* �����`��f�[�^.
*/
struct RenderingPart {
	GLsizei size; ///< �`�悷��C���f�b�N�X��.
	GLvoid* offset; ///< �`��J�n�C���f�b�N�X�̃o�C�g�I�t�Z�b�g.
};

/**
* RenderingPart���쐬����.
*
* @param size �`�悷��C���f�b�N�X��.
* @param offset �`��J�n�C���f�b�N�X�̃I�t�Z�b�g(�C���f�b�N�X�P��).
*
* @return �쐬���������`��I�u�W�F�N�g. 
*/
constexpr RenderingPart MakeRenderingPart(GLsizei size, GLsizei offset) {
	return{ size, reinterpret_cast<GLvoid*>(offset * sizeof(GLuint)) };
}

/**
* �����`��f�[�^���X�g.
*/
static const RenderingPart renderingParts[] = {
	MakeRenderingPart(12, 0),
	MakeRenderingPart(6, 12),
};

/**
* Vertex Buffer Object ���쐬����.
*
* @param size ���_�f�[�^�̃T�C�Y.
* @param data ���_�f�[�^�ւ̃|�C���^.
*
* @return �쐬���� VBO.
*/
GLuint CreateVBO(GLsizeiptr size, const GLvoid* data) {
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return vbo;
}


/**
* ���_�A�g���r���[�g��ݒ肷��
* Index Buffer Object���쐬����
*
* @param size �C���f�b�N�X�f�[�^�̃T�C�Y
* @param data �C���f�b�N�X�f�[�^�ւ̃|�C���^
*
* @return �쐬����IBO
*/
GLuint CreateIBO(GLsizeiptr size, const GLvoid* data){
	GLuint ibo = 0;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return ibo;
}


/**
* ���_�A�g���r���[�g��ݒ肷��.
*
* @param index ���_�A�g���r���[�g�̃C���f�b�N�X.
* @param cls   ���_�f�[�^�^��.
* @param mbr   ���_�A�g���r���[�g�ɐݒ肷�� cls �̃����o�ϐ���.
*/
#define SetVertexAttribPointer(index, cls, mbr) SetVertexAttribPointerI( \
	index, \
	sizeof(cls::mbr) / sizeof(float), \
	sizeof(cls), \
	reinterpret_cast<GLvoid*>(offsetof(cls, mbr))) 

void SetVertexAttribPointerI(GLuint index, GLint size, GLsizei stride, const GLvoid* pointer) {
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, pointer);
}

/**
* Vertex Array Object ���쐬����.
*
* @param vbo VAO �Ɋ֘A�t������ VBO.
* @param ibo VAO �Ɋ֘A�t������ IBO.
*
* @return �쐬���� VAO.
*/
GLuint CreateVAO(GLuint vbo, GLuint ibo) {
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	SetVertexAttribPointer(0, Vertex, position);
	SetVertexAttribPointer(1, Vertex, color);
	SetVertexAttribPointer(2, Vertex, texCoord);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindVertexArray(0);
	return vao;
}

/**
* Uniform Block Object���쐬����.
*
* @param size Uniform Block�̃T�C�Y.
* @param data Uniform Block�ɓ]������f�[�^�ւ̃|�C���^.
*
* @return �쐬����UBO.
*/
GLuint CreateUBO(GLsizeiptr size, const GLvoid* data = nullptr)
{
	GLuint ubo;
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return ubo;
}

/**
* GLFW ����̃G���[�񍐂���������.
*
* @param error �G���[�ԍ�.
* @param desc  �G���[�̓��e.
*/
void ErrorCallback(int error, const char* desc) {
	std::cerr << "ERROR: " << desc << std::endl;
}

/// �G���g���[�|�C���g
int main() {

	// �G���[�R�[���o�b�N
	glfwSetErrorCallback(ErrorCallback);

	// GLFWEW�̏�����
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	if (!window.Init(800, 600, "OpenGL Tutorial")) {
		return 1;
	}

	// �`��f�o�C�X�A�Ή��o�[�W�����o��
	const GLubyte* renderer = glGetString(GL_RENDERER);
	std::cout << "Renderer: " << renderer << std::endl;
	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "Version: " << version << std::endl;

	// �I�u�W�F�N�g�̍쐬
	const GLuint vbo = CreateVBO(sizeof(vertices), vertices);
	const GLuint ibo = CreateIBO(sizeof(indices), indices);
	const GLuint vao = CreateVAO(vbo, ibo);
	const UniformBufferPtr uboVertex = UniformBuffer::Create(sizeof(VertexData), 0, "VertexData");
	// ���C�g�f�[�^�p�� UBO ���쐬
	const UniformBufferPtr uboLight = UniformBuffer::Create(sizeof(LightData), 1, "LightData");
	// �F�ϊ��s��p��UBO���쐬
	const UniformBufferPtr uboPostEffect = UniformBuffer::Create(sizeof(PostEffectData), 2, "PostEffectData");
	
	const GLuint ubo = CreateUBO(sizeof(VertexData));

	// �V�F�[�_�[��ǂݍ���
	// �`���[�g���A���V�F�[�_�[
	const Shader::ProgramPtr progTutorial = Shader::Program::Create("Res/Tutorial.vert", "Res/Tutorial.frag");
	// �J���[�t�B���^�[�V�F�[�_�[
	//const Shader::ProgramPtr progColorFilter = Shader::Program::Create("Res/ColorFilter.vert", "Res/ColorFilter.frag");
	// �|�X�^�[���V�F�[�_�[
	const Shader::ProgramPtr progColorFilter = Shader::Program::Create("Res/Posterization.vert", "Res/Posterization.frag");
	
	if (!vbo || !ibo || !vao || !uboVertex || !uboLight || !progTutorial || !progColorFilter) {
		return 1;
	}
	progTutorial->UniformBlockBinding("VertexData", 0);
	progTutorial->UniformBlockBinding("LightData", 1);

	/*
	const GLuint uboIndex = glGetUniformBlockIndex(shaderProgram, "VertexData");
	if (uboIndex == GL_INVALID_INDEX) {
		return 1;
	}
	glUniformBlockBinding(shaderProgram, uboIndex, 0); 
	*/
	// �e�N�X�`���f�[�^.
	static const uint32_t textureData[] = {
		0xffffffff, 0xffcccccc, 0xffffffff, 0xffcccccc, 0xffffffff,
		0xff888888, 0xffffffff, 0xff888888, 0xffffffff, 0xff888888,
		0xffffffff, 0xff444444, 0xffffffff, 0xff444444, 0xffffffff,
		0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000,
		0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff,
	}; 

	//TexturePtr tex = Texture::Create(5, 5, GL_RGBA8, GL_RGBA, textureData);
	TexturePtr tex = Texture::LoadFromFile("Res/Sample.bmp");
	if (!tex) {
		return 1;
	}

	// �[�x�o�b�t�@�g�p
	glEnable(GL_DEPTH_TEST);

	// �I�t�X�N���[���o�b�t�@�쐬
	const OffscreenBufferPtr offscreen = OffscreenBuffer::Create(800, 600);

	// ���C�����[�v
	while (!window.ShouldClose()) {
		// �`���S�ĕύX�����I�t�X�N���[���o�b�t�@�ɑ΂��čs����
		glBindFramebuffer(GL_FRAMEBUFFER, offscreen->GetFramebuffer());

		glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		static float degree = 0.0f; degree += 0.1f;
		if (degree >= 360.0f) {
			degree -= 360.0f;
		}

		// ���_�𓮂���
		const glm::vec3 viewPos = glm::rotate(glm::mat4(), glm::radians(degree), glm::vec3(0, 1, 0)) * glm::vec4(2, 3, 3, 1);

		// ���_�f�[�^�̕`��
		progTutorial->UseProgram();

		// UBO �Ƀf�[�^��]��
		const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		const glm::mat4x4 matView = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		VertexData vertexData;
		vertexData.matMVP = matProj * matView;
		//vertexData.lightPosition = glm::vec4(1, 1, 1, 1);
		//vertexData.lightColor = glm::vec4(2, 2, 2, 1);
		//vertexData.ambientColor = glm::vec4(0.05f, 0.1f, 0.2f, 1);
		// UBO �փf�[�^��]��
		uboVertex->BufferSubData(&vertexData);

		LightData lightData;
		lightData.ambientColor= glm::vec4(0.05f, 0.1f, 0.2f, 1);
		lightData.light[0].position = glm::vec4(1, 1, 1, 1);
		lightData.light[0].color = glm::vec4(2, 2, 2, 1);
		lightData.light[1].position = glm::vec4(-0.2f, 0, 0.6f, 1);
		lightData.light[1].color = glm::vec4(0.125f, 0.125f, 0.05f, 1);
		uboLight->BufferSubData(&lightData);

		// ���W�ϊ��s����쐬���Auniform�ϐ��ɓ]������
		/*
		const GLint matMVPLoc = glGetUniformLocation(shaderProgram, "matMVP");
		if (matMVPLoc >= 0) {
			const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
			const glm::mat4x4 matView = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			const glm::mat4x4 matMVP = matProj * matView;
			glUniformMatrix4fv(matMVPLoc, 1, GL_FALSE, &matMVP[0][0]);
		}*/

		// �e�N�X�`���ݒ�
		progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, tex->Id());

		glBindVertexArray(vao);

		//�C���f�b�N�X�f�[�^�ɂ��`��
		glDrawElements(
			GL_TRIANGLES, renderingParts[0].size,
			GL_UNSIGNED_INT, renderingParts[0].offset
		);

		// �I�t�X�N���[���o�b�t�@���g���ăo�b�N�o�b�t�@��`�悷��
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.5f, 0.3f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// ���m�g�[���V�F�[�_�[�g�p
		progColorFilter->UseProgram();
		progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());

		PostEffectData postEffect;
		postEffect.matColor[0] = glm::vec4(0.393f, 0.349f, 0.272f, 0);
		postEffect.matColor[1] = glm::vec4(0.769f, 0.686f, 0.534f, 0);
		postEffect.matColor[2] = glm::vec4(0.189f, 0.168f, 0.131f, 0);
		postEffect.matColor[3] = glm::vec4(0, 0, 0, 1);
		uboPostEffect->BufferSubData(&postEffect);
		
		// ������
		//vertexData = {};
		//vertexData.ambientColor = glm::vec4(1, 1, 1, 1);
		// UBO �Ƀf�[�^��]��
		//uboVertex->BufferSubData(&vertexData);
		//lightData = {};
		//lightData.ambientColor= glm::vec4(1);
		//uboLight->BufferSubData(&lightData);

		glDrawElements(
			GL_TRIANGLES, renderingParts[1].size,
			GL_UNSIGNED_INT, renderingParts[1].offset
		);

		window.SwapBuffers();
	}

	// �I�u�W�F�N�g�̍폜
	//glDeleteProgram(shaderProgram);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	// �I������
	glfwTerminate();

	return 0;
} 
