/**
* @file	Main.cpp
*/

#include "GLFWEW.h"
#include "Texture.h"
#include "Shader.h"
#include "OffscreenBuffer.h"
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
	const GLuint ubo = CreateUBO(sizeof(VertexData));
	// �V�F�[�_�[��ǂݍ���
	const GLuint shaderProgram = Shader::CreateProgramFromFile("Res/Tutorial.vert", "Res/Tutorial.flag");
	if (!vbo || !ibo || !vao || !ubo || !shaderProgram) {
		return 1;
	}

	const GLuint uboIndex = glGetUniformBlockIndex(shaderProgram, "VertexData");
	if (uboIndex == GL_INVALID_INDEX) {
		return 1;
	}
	glUniformBlockBinding(shaderProgram, uboIndex, 0); 

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
		glUseProgram(shaderProgram);

		// UBO �Ƀf�[�^��]��
		const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		const glm::mat4x4 matView = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		VertexData vertexData;
		vertexData.matMVP = matProj * matView;
		vertexData.lightPosition = glm::vec4(1, 1, 1, 1);
		vertexData.lightColor = glm::vec4(2, 2, 2, 1);
		vertexData.ambientColor = glm::vec4(0.05f, 0.1f, 0.2f, 1);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VertexData), &vertexData);


		// ���W�ϊ��s����쐬���Auniform�ϐ��ɓ]������
		const GLint matMVPLoc = glGetUniformLocation(shaderProgram, "matMVP");
		if (matMVPLoc >= 0) {
			const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
			const glm::mat4x4 matView = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			const glm::mat4x4 matMVP = matProj * matView;
			glUniformMatrix4fv(matMVPLoc, 1, GL_FALSE, &matMVP[0][0]);
		}

		// �T���v���[�̈ʒu���擾
		const GLint colorSamplerLoc = glGetUniformLocation(shaderProgram, "colorSampler");

		// �T���v���[�ƃe�N�X�`�������т��� 
		if (colorSamplerLoc >= 0) {
			glUniform1i(colorSamplerLoc, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex->Id());
		}

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
		if (colorSamplerLoc >= 0) {
			glBindTexture(GL_TEXTURE_2D, offscreen->GetTexutre());
		}
		
		// ������
		vertexData = {};
		vertexData.ambientColor = glm::vec4(1, 1, 1, 1);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VertexData), &vertexData);

		glDrawElements(
			GL_TRIANGLES, renderingParts[1].size,
			GL_UNSIGNED_INT, renderingParts[1].offset
		);

		window.SwapBuffers();
	}

	// �I�u�W�F�N�g�̍폜
	glDeleteProgram(shaderProgram);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	// �I������
	glfwTerminate();

	return 0;
} 
