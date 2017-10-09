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
* ゲームエンジンクラス.
*/
class GameEngine{
public:
	
	typedef std::function<void(double)> UpdateFuncType;/// ゲーム状態を更新する関数の型.
	/// カメラデータ.
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
	bool LoadTextureFromFile(const char* filename);
	Entity::Entity* AddEntity(int groupId, const glm::vec3& pos, const char* meshName,
		const char* texName, Entity::Entity::UpdateFuncType func, const char* shader = nullptr);
	void RemoveEntity(Entity::Entity*);
	void Light(int index, const Uniform::PointLight& light);
	const Uniform::PointLight& Light(int index) const;
	void AmbientLight(const glm::vec4& color);
	const glm::vec4& AmbientLight() const;
	void Camera(const CameraData& cam);
	const CameraData& Camera() const;
	std::mt19937& Rand();
	const GamePad& GetGamePad() const;

	bool InitAudio(const char* acfPath, const char* acbPath, const char* awbPath, const char* dspBusName);
	void PlayAudio(int playerId, int cueId);
	void StopAudio(int playerId);

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

	//<--- ここにメンバ関数を追加する --->
	GLuint vbo = 0;
	GLuint ibo = 0;
	GLuint vao = 0;
	UniformBufferPtr uboLight;
	UniformBufferPtr uboPostEffect;
	std::unordered_map<std::string, Shader::ProgramPtr> shaderMap;
	OffscreenBufferPtr offscreen;

	std::unordered_map<std::string, TexturePtr> textureBuffer;
	Mesh::BufferPtr meshBuffer;
	Entity::BufferPtr entityBuffer;
	Font::Renderer fontRenderer;
	Uniform::LightData lightData;
	CameraData camera;
	std::mt19937 rand;
	// 様々な変数を格納できる機能を追加(得点を保存する変数とか).
	std::unordered_map<std::string, double> userNumbers;

private:
	//<--- ここにメンバ関数を追加する --->
};

#endif // GAMEENGINE_H_INCLUDED