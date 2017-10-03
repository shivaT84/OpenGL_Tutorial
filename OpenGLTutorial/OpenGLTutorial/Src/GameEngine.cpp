/**
* @file GameEngine.cpp
*/
#include "GameEngine.h"
#include "GLFWEW.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <time.h>


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
	{ { -0.5f,-0.3f, 0.5f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
	{ { 0.3f,-0.3f, 0.5f  },{ 0.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 0.3f, 0.5f, 0.5f  },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
	{ { -0.5f, 0.5f, 0.5f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },

	{ { -0.3f, 0.3f, 0.1f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
	{ { -0.3f,-0.5f, 0.1f },{ 0.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
	{ { 0.5f,-0.5f, 0.1f  },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 0.5f,-0.5f, 0.1f  },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, 0.3f, 0.1f  },{ 1.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
	{ { -0.3f, 0.3f, 0.1f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },

	{ { -1.0f,-1.0f, 0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 1.0f,-1.0f, 0.5f  },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
	{ { 1.0f, 1.0f, 0.5f  },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
	{ { -1.0f, 1.0f, 0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
};

/// �C���f�b�N�X�f�[�^.
const GLuint indices[] = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 7, 8, 9,
	10, 11, 12, 12, 13, 10,
};


/**
* �����`��f�[�^.
*/
struct RenderingData {
	GLsizei size; ///< �`�悷��C���f�b�N�X��.
	GLvoid* offset; ///< �`��J�n�C���f�b�N�X�̃o�C�g�I�t�Z�b�g.
};

/**
* RenderingData���쐬����.
*
* @param size �`�悷��C���f�b�N�X��.
* @param offset �`��J�n�C���f�b�N�X�̃I�t�Z�b�g(�C���f�b�N�X�P��).
*
* @return �쐬���������`��I�u�W�F�N�g.
*/
constexpr RenderingData MakeRenderingData(GLsizei size, GLsizei offset) {
	return{ size, reinterpret_cast<GLvoid*>(offset * sizeof(GLuint)) };
}

/**
* �����`��f�[�^���X�g.
*/
static const RenderingData renderingData[] = {
	MakeRenderingData(12, 0),
	MakeRenderingData(6, 12),
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
GLuint CreateIBO(GLsizeiptr size, const GLvoid* data) {
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
* �Q�[���G���W���̃C���X�^���X���擾����.
*
* @return �Q�[���G���W���̃C���X�^���X.
*/
GameEngine& GameEngine::Instance() {
	static GameEngine instance;
	return instance;
}

/**
* �Q�[���G���W��������������.
*
* @param w     �E�B���h�E�̕`��͈͂̕�(�s�N�Z��).
* @param h     �E�B���h�E�̕`��͈͂̍���(�s�N�Z��).
* @param title �E�B���h�E�^�C�g��(UTF-8��0�I�[������).
*
* @retval true  ����������.
* @retval false ���������s.
*
* Update, Render�֐��Ȃǂ��Ăяo���O�ɁA��x�����Ăяo���Ă����K�v������.
* ��x�������ɐ�������ƁA�Ȍ�̌Ăяo���ł͂Ȃɂ�������true��Ԃ�.
*/
bool GameEngine::Init(int w, int h, const char* title){
	if (isInitialized) {
		return true;
	}
	if (!GLFWEW::Window::Instance().Init(w, h, title)) {
		return false;
	}

	vbo = CreateVBO(sizeof(vertices), vertices);
	ibo = CreateIBO(sizeof(indices), indices);
	vao = CreateVAO(vbo, ibo);
	uboLight = UniformBuffer::Create(sizeof(Uniform::LightData), 1, "LightingData");
	uboPostEffect = UniformBuffer::Create(sizeof(Uniform::PostEffectData), 2, "PostEffectData");
	progTutorial = Shader::Program::Create("Res/Tutorial.vert", "Res/Tutorial.frag");

	progColorFilter = Shader::Program::Create("Res/ColorFilter.vert", "Res/ColorFilter.frag");
	offscreen = OffscreenBuffer::Create(800, 600);

	if (!vbo || !ibo || !vao || !uboLight || !uboPostEffect || !progTutorial || !progColorFilter || !offscreen) {
		std::cerr << "ERROR: GameEngine �̏������Ɏ��s" << std::endl;
		return false;
	}
	progTutorial->UniformBlockBinding("VertexData", 0);
	progTutorial->UniformBlockBinding("LightData", 1);
	progColorFilter->UniformBlockBinding("PostEffectData", 2);

	meshBuffer = Mesh::Buffer::Create(10 * 1024, 30 * 1024);
	if (!meshBuffer) {
		std::cerr << "ERROR: GameEngine �̏������Ɏ��s" << std::endl;
		return false;
	}
	entityBuffer = Entity::Buffer::Create(1024, sizeof(Uniform::VertexData), 0, "VertexData");
	if (!entityBuffer) {
		std::cerr << "ERROR: GameEngine �̏������Ɏ��s" << std::endl;
		return false;
	}
	rand.seed(std::random_device()());

	isInitialized = true;
	return true;
}

/**
* �Q�[�������s����.
*/
void GameEngine::Run(){
	const double delta = 1.0 / 60.0;
	GLFWEW::Window& window = GLFWEW::Window::Instance();

	while (!window.ShouldClose()) {
		window.UpdateGamePad();
		Update(delta);
		Render();
		window.SwapBuffers();
	}
}

/**
* ��ԍX�V�֐���ݒ肷��.
*
* @param func �ݒ肷��X�V�֐�.
*/
void GameEngine::UpdateFunc(const UpdateFuncType& func){
	updateFunc = func;
}

/**
* ��ԍX�V�֐����擾����.
*
* @return �ݒ肳��Ă���X�V�֐�.
*/
const GameEngine::UpdateFuncType& GameEngine::UpdateFunc() const{
	return updateFunc;
}

/**
* �e�N�X�`����ǂݍ���.
*
* @param filename �e�N�X�`���t�@�C����.
*
* @retval true  �ǂݍ��ݐ���.
* @retval false �ǂݍ��ݎ��s.
*/
bool GameEngine::LoadTextureFromFile(const char* filename){
	const auto itr = textureBuffer.find(filename);
	if (itr != textureBuffer.end()) {
		return true;
	}
	TexturePtr texture = Texture::LoadFromFile(filename);
	if (!texture) {
		return false;
	}
	textureBuffer.insert(std::make_pair(std::string(filename), texture));
	return true;
}

/**
* ���b�V����ǂݍ���.
*
* @param filename ���b�V���t�@�C����.
*
* @retval true  �ǂݍ��ݐ���.
* @retval false �ǂݍ��ݎ��s.
*/
bool GameEngine::LoadMeshFromFile(const char* filename){
	return meshBuffer->LoadMeshFromFile(filename);
}

/**
* �G���e�B�e�B��ǉ�����.
*
* @param position �G���e�B�e�B�̍��W.
* @param meshName �G���e�B�e�B�̕\���Ɏg�p���郁�b�V����.
* @param texName  �G���e�B�e�B�̕\���Ɏg���e�N�X�`���t�@�C����.
* @param func     �G���e�B�e�B�̏�Ԃ��X�V����֐�(�܂��͊֐��I�u�W�F�N�g).
*
* @return �ǉ������G���e�B�e�B�ւ̃|�C���^.
*         ����ȏ�G���e�B�e�B��ǉ��ł��Ȃ��ꍇ��nullptr���Ԃ����.
*         ��]��g�嗦�͂��̃|�C���^�o�R�Őݒ肷��.
*         �Ȃ��A���̃|�C���^���A�v���P�[�V�������ŕێ�����K�v�͂Ȃ�.
*/
Entity::Entity* GameEngine::AddEntity(const glm::vec3& pos, const char* meshName, const char* texName, Entity::Entity::UpdateFuncType func){
	const Mesh::MeshPtr& mesh = meshBuffer->GetMesh(meshName);
	const TexturePtr& tex = textureBuffer.find(texName)->second;
	return entityBuffer->AddEntity(pos, mesh, tex, progTutorial, func);
}

/**
*�@�G���e�B�e�B���폜����.
*
* @param �폜����G���e�B�e�B�̃|�C���^.
*/
void GameEngine::RemoveEntity(Entity::Entity* e){
	entityBuffer->RemoveEntity(e);
}

/**
* ���C�g��ݒ肷��.
*
* @param indes  �ݒ肷�郉�C�g�̃C���f�b�N�X.
* @param light  ���C�g�p�����[�^.
*/
void GameEngine::Light(int index, const Uniform::PointLight& light){
	if (index < 0 || index >= Uniform::maxLightCount) {
		std::cerr << "WARNING: '" << index << "'�͕s���ȃ��C�g�C���f�b�N�X�ł�" << std::endl;
		return;
	}
	lightData.light[index] = light;
}

/**
* ���C�g���擾����.
*
* @param index �擾���郉�C�g�̃C���f�b�N�X.
*
* @return ���C�g�p�����[�^.
*/
const Uniform::PointLight& GameEngine::Light(int index) const{
	if (index < 0 || index >= Uniform::maxLightCount) {
		std::cerr << "WARNING: '" << index << "'�͕s���ȃ��C�g�C���f�b�N�X�ł�" << std::endl;
		static const Uniform::PointLight dummy;
		return dummy;
	}
	return lightData.light[index];
}

/**
* ������ݒ肷��.
*
* @param color �����̖��邳.
*/
void GameEngine::AmbientLight(const glm::vec4& color){
	lightData.ambientColor = color;
}

/**
* �������擾����.
*
* @return �����̖��邳.
*/
const glm::vec4& GameEngine::AmbientLight() const{
	return lightData.ambientColor;
}

/**
* ���_�̈ʒu�Ǝp����ݒ肷��.
*
* @param cam �ݒ肷��J�����f�[�^.
*/
void GameEngine::Camera(const CameraData& cam){
	camera = cam;
}

/**
* ���_�̈ʒu�Ǝp�����擾����.
*
* @return �J�����f�[�^.
*/
const GameEngine::CameraData& GameEngine::Camera() const{
	return camera;
}

/**
* �����I�u�W�F�N�g���擾����.
*
* @return �����I�u�W�F�N�g.
*/
std::mt19937& GameEngine::Rand(){
	return rand;
}

/**
* �Q�[���p�b�h�̏�Ԃ��擾����.
*/
const GamePad& GameEngine::GetGamePad() const {
	return GLFWEW::Window::Instance().GetGamePad();
}

/**
* �f�X�g���N�^.
*/
GameEngine::~GameEngine(){
	if (vao) {
		glDeleteVertexArrays(1, &vao);
	}
	if (ibo) {
		glDeleteBuffers(1, &ibo);
	}
	if (vbo) {
		glDeleteBuffers(1, &vbo);
	}
}

/**
* �Q�[���̏�Ԃ��X�V����.
*
* @param delta �O��̍X�V����̌o�ߎ���(�b).
*/
void GameEngine::Update(double delta){
	if (updateFunc) {
		updateFunc(delta);
	}

	const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 200.0f);
	const glm::mat4x4 matView = glm::lookAt(camera.position, camera.target, camera.up);
	entityBuffer->Update(delta, matView, matProj);
}

/**
* �Q�[���̏�Ԃ�`�悷��.
*/
void GameEngine::Render() const{

	glBindFramebuffer(GL_FRAMEBUFFER, offscreen->GetFramebuffer());
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, 800, 600);
	glScissor(0, 0, 800, 600);
	glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	uboLight->BufferSubData(&lightData);
	entityBuffer->Draw(meshBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBindVertexArray(vao);
	progColorFilter->UseProgram();
	Uniform::PostEffectData postEffect;
	uboPostEffect->BufferSubData(&postEffect);
	progColorFilter->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());
	glDrawElements(GL_TRIANGLES, renderingData[1].size, GL_UNSIGNED_INT, renderingData[1].offset);
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