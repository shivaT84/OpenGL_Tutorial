/**
* @file	Main.cpp
*/

#include "GameEngine.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>


/**
* 敵の円盤の状態を更新する.
*/
struct UpdateToroid {

	void operator()(Entity::Entity& entity, void* ubo, double delta, const glm::mat4& matView, const glm::mat4& matProj) {
		// 範囲外に出たら削除する.
		const glm::vec3 pos = entity.Position();
		if (std::abs(pos.x) > 40.0f || std::abs(pos.z) > 40.0f) {
			GameEngine::Instance().RemoveEntity(&entity);
			return;
		}

		// 円盤を回転させる.
		float rot = glm::angle(entity.Rotation());
		rot += glm::radians(15.0f) * static_cast<float>(delta);
		if (rot > glm::pi<float>() * 2.0f) {
			rot -= glm::pi<float>() * 2.0f;
		}
		entity.Rotation(glm::angleAxis(rot, glm::vec3(0, 1, 0))); 

		// 頂点シェーダーのパラメータを UBO にコピーする.
		Uniform::VertexData data;
		data.matModel = entity.TRSMatrix();
		data.matNormal = glm::mat4_cast(entity.Rotation());
		data.matMVP = matProj * matView * data.matModel;
		memcpy(ubo, &data, sizeof(Uniform::VertexData));
	} 
};

/**
* 自機の更新
*/
struct UpdatePlayer {
	void operator()(Entity::Entity& entity, void* ubo, double delta, const glm::mat4& matView, const glm::mat4& matProj) {

		GameEngine& game = GameEngine::Instance();
		const GamePad gamepad = game.GetGamePad();
		glm::vec3 vec;
		float rotZ = 0;
		if (gamepad.buttons & GamePad::DPAD_LEFT) {
			vec.x = 1;
			rotZ = -glm::radians(30.0f);
		}else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
			vec.x = -1;
			rotZ = glm::radians(30.0f);
		}

		if (gamepad.buttons & GamePad::DPAD_UP) {
			vec.y = 1;
		}else if (gamepad.buttons & GamePad::DPAD_DOWN) {
			vec.y = -1;
		}

		if (vec.x || vec.z) {
			vec = glm::normalize(vec) * 2.0f;
		}
		entity.Velocity(vec);
		entity.Rotation(glm::quat(glm::vec3(0, 0, rotZ)));

		glm::vec3 pos = entity.Position();
		pos = glm::min(glm::vec3(11, 100, 20), glm::max(pos, glm::vec3(-11, -100, 1)));
		entity.Position(pos);

		Uniform::VertexData data;
		data.matModel = entity.TRSMatrix();
		data.matNormal = glm::mat4_cast(entity.Rotation());
		data.matMVP = matProj * matView * data.matModel;
		memcpy(ubo, &data, sizeof(Uniform::VertexData));
	}
};

/**
* ゲーム状態の更新.
*/
struct Update {
	void operator()(double delta) {
		GameEngine& game = GameEngine::Instance();

		if (!pPlayer) {
			pPlayer = game.AddEntity(glm::vec3(0, 0, 2), "Aircraft", "Res/Player.bmp", UpdatePlayer());
		}

		game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });
		game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
		game.Light(0, { glm::vec4(40, 100, 10, 1), glm::vec4(12000, 12000, 12000, 1) });
		std::uniform_int_distribution<> distributerX(-12, 12);
		std::uniform_int_distribution<> distributerZ(40, 44);
		interval -= delta;
		if (interval <= 0) {
			const std::uniform_real_distribution<> rndInterval(8.0, 16.0);
			const std::uniform_int_distribution<> rndAddingCount(1, 5);
			for (int i = rndAddingCount(game.Rand()); i > 0; --i) {
				const glm::vec3 pos(distributerX(game.Rand()), 0, distributerZ(game.Rand()));
				if (Entity::Entity* p = game.AddEntity(pos, "Toroid", "Res/Toroid.bmp", UpdateToroid())) {
					p->Velocity(glm::vec3(pos.x < 0 ? 1.0f : -1.0f, 0, -4));
				}
			}
			interval = rndInterval(game.Rand());
		}
	}

	double interval = 0;
	Entity::Entity* pPlayer = nullptr;
};

/**
* Uniform Block Objectを作成する.
*
* @param size Uniform Blockのサイズ.
* @param data Uniform Blockに転送するデータへのポインタ.
*
* @return 作成したUBO.
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

/// エントリーポイント
int main() {
	GameEngine& game = GameEngine::Instance();
	if (!game.Init(800, 600, "OpenGL Tutorial")) {
		return 1;
	}
	game.LoadTextureFromFile("Res/Toroid.bmp");
	game.LoadTextureFromFile("Res/Player.bmp");
	game.LoadMeshFromFile("Res/Toroid.fbx");
	game.LoadMeshFromFile("Res/Player.fbx");
	game.UpdateFunc(Update());   game.Run();
	return 0;
}
