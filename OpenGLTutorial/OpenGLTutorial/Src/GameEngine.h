/**
* @file GameEngine.h
*/

#ifndef GAMEENGINE_H_INCLUDED
#define GAMEENGINE_H_INCLUDED

#include <GL/glew.h>
#include "UniformBuffer.h"
#include "OffscreenBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "Entity.h"
#include "Uniform.h"
#include "GamePad.h"
#include "Font.h"
#include <glm/glm.hpp>
#include <functional>
#include <random>

/**
* �Q�[���G���W���N���X.
*/
class GameEngine{
public:
	
	typedef std::function<void(double)> UpdateFuncType;/// �Q�[����Ԃ��X�V����֐��̌^.
	/// �J�����f�[�^.
	struct CameraData {
		glm::vec3 position;
		glm::vec3 target;
		glm::vec3 up;
	};

	static GameEngine& Instance();
	bool Init(int w, int h, const char* title);
	void Run();
	void UpdateFunc(const UpdateFuncType& func);
	const UpdateFuncType& UpdateFunc() const;

	bool LoadMeshFromFile(const char* filename);
	//const Mesh::MeshPtr& GetMesh(const char* name);
	bool LoadTextureFromFile(const char* filename);
	//const TexturePtr& GetTexture(const char* filename) const;
	Entity::Entity* AddEntity(int groupId, const glm::vec3& pos, const char* meshName,
		const char* texName, Entity::Entity::UpdateFuncType func);
	void RemoveEntity(Entity::Entity*);
	void Light(int index, const Uniform::PointLight& light);
	const Uniform::PointLight& Light(int index) const;
	void AmbientLight(const glm::vec4& color);
	const glm::vec4& AmbientLight() const;
	void Camera(const CameraData& cam);
	const CameraData& Camera() const;
	std::mt19937& Rand();
	const GamePad& GetGamePad() const;
	void CollisionHandler(int gid0, int gid1, Entity::CollisionHandlerType handler);
	const Entity::CollisionHandlerType& CollisionHandler(int gid0, int gid1) const;
	void ClearCollisionHandlerList();

	const TexturePtr& GetTexture(const char* filename) const {
		static const TexturePtr dummy;
		auto itr = textureBuffer.find(filename);
		return itr != textureBuffer.end() ? itr->second : dummy;
	}
	bool LoadFontFromFile(const char* filename) {
		return fontRenderer.LoadFromFile(filename);
	}
	bool AddString(const glm::vec2& pos, const char* str) {
		return fontRenderer.AddString(pos, str);
	}
	void FontScale(const glm::vec2& scale) { fontRenderer.Scale(scale); }
	void FontColor(const glm::vec4& color) { fontRenderer.Color(color); }
	double& UserVariable(const char* name) { return userNumbers[name]; }
	
private:
	GameEngine() = default;
	~GameEngine();
	GameEngine(const GameEngine&) = delete;
	GameEngine& operator=(const GameEngine&) = delete;
	void Update(double delta);
	void Render() const;

private:
	bool isInitialized = false;
	UpdateFuncType updateFunc;

	//<--- �����Ƀ����o�֐���ǉ����� --->
	GLuint vbo = 0;
	GLuint ibo = 0;
	GLuint vao = 0;
	UniformBufferPtr uboLight;
	UniformBufferPtr uboPostEffect;
	Shader::ProgramPtr progTutorial;
	Shader::ProgramPtr progColorFilter;
	OffscreenBufferPtr offscreen;
	/*
	Shader::ProgramPtr progPostEffect;
	Shader::ProgramPtr progBloom1st;
	Shader::ProgramPtr progComposition;
	Shader::ProgramPtr progSimple;
	Shader::ProgramPtr progLensFlare;
	Shader::ProgramPtr progNonLighting;
	
	static const int bloomBufferCount = 6;
	OffscreenBufferPtr offBloom[bloomBufferCount];
	OffscreenBufferPtr offAnamorphic[2];
	*/
	std::unordered_map<std::string, TexturePtr> textureBuffer;
	Mesh::BufferPtr meshBuffer;
	Entity::BufferPtr entityBuffer;
	Font::Renderer fontRenderer;
	Uniform::LightData lightData;
	CameraData camera;
	std::mt19937 rand;
	// �l�X�ȕϐ����i�[�ł���@�\��ǉ�(���_��ۑ�����ϐ��Ƃ�).
	std::unordered_map<std::string, double> userNumbers;

private:
	//<--- �����Ƀ����o�֐���ǉ����� --->
};

//void DefaultUpdateVertexData(Entity::Entity& e, void* ubo, double, const glm::mat4& matView, const glm::mat4& matProj);

#endif // GAMEENGINE_H_INCLUDED