/**
* @file MainGameState.cpp
*/
#include "GameState.h"
#include "GameEngine.h"
#include "../Res/Audio/SampleCueSheet.h"
#include <algorithm>

namespace GameState {

	/// 衝突形状リスト.
	static const Entity::CollisionData collisionDataList[] = {
		{},//背景用のダミーデータ.
		{ glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
		{ glm::vec3(-0.5f, -0.5f, -1.0f), glm::vec3(0.5f, 0.5f, 1.0f) },
		{ glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
		{ glm::vec3(-0.25f, -0.25f, -0.25f), glm::vec3(0.25f, 0.25f, 0.25f) },
	};

	/**
	* 敵の円盤の状態を更新する.
	*/
	struct UpdateToroid {

		void operator()(Entity::Entity& entity, double delta) {
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
		}
	};

	/**
	* 自機の弾の更新.
	*/
	struct UpdatePlayerShot {
		void operator()(Entity::Entity& entity, double delta) {
			const glm::vec3 pos = entity.Position();
			if (std::abs(pos.x) > 40 || pos.z < -4 || pos.z > 40) {
				entity.Destroy();
				return;
			}
		}
	};

	/**
	* @desc	爆発の更新.
	* @tips	時間経過に応じて大きさ、色、角度を変化させてゆき、一定時間が経過したら削除する
	*/
	struct UpdateBlast {
		void operator()(Entity::Entity& entity, double delta) {
			// 一定時間が経過したら削除
			timer += delta;
			if (timer >= 0.5) {
				entity.Destroy();
				return;
			}
			const float variation = static_cast<float>(timer * 4); // 変化量.
			// 大きさは1倍からスタートして線形に拡大率を上げていき、消滅直前におよそ3倍になるように設定
			entity.Scale(glm::vec3(static_cast<float>(1 + variation))); // 徐々に拡大する.

			// 時間経過で色と透明度を変化させる.
			static const glm::vec4 color[] = {
				glm::vec4(1.0f, 1.0f, 0.75f, 1),
				glm::vec4(1.0f, 0.5f, 0.1f, 1),
				glm::vec4(0.25f, 0.1f, 0.1f, 0),
			};
			const glm::vec4 col0 = color[static_cast<int>(variation)];
			const glm::vec4 col1 = color[static_cast<int>(variation) + 1];
			const glm::vec4 newColor = glm::mix(col0, col1, std::fmod(variation, 1));
			entity.Color(newColor);

			// Y軸まわりを秒間60度の速度で回転させる.
			glm::vec3 euler = glm::eulerAngles(entity.Rotation());
			euler.y += glm::radians(60.0f) * static_cast<float>(delta);
			entity.Rotation(glm::quat(euler));
		}

		double timer = 0;
	};

	/**
	* 自機の更新
	*/
	struct UpdatePlayer {
		void operator()(Entity::Entity& entity, double delta) {

			// perator()の座標を更新する処理
			GameEngine& game = GameEngine::Instance();
			const GamePad gamepad = game.GetGamePad();
			glm::vec3 vec;
			float rotZ = 0;
			if (gamepad.buttons & GamePad::DPAD_LEFT) {
				vec.x = 1;
				rotZ = -glm::radians(30.0f);
			}
			else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
				vec.x = -1;
				rotZ = glm::radians(30.0f);
			}

			if (gamepad.buttons & GamePad::DPAD_UP) {
				vec.y = 1;
			}
			else if (gamepad.buttons & GamePad::DPAD_DOWN) {
				vec.y = -1;
			}

			if (vec.x || vec.z) {
				vec = glm::normalize(vec) * 15.0f;//移動速度
			}
			entity.Velocity(vec);
			entity.Rotation(glm::quat(glm::vec3(0, 0, rotZ)));

			glm::vec3 pos = entity.Position();
			pos = glm::min(glm::vec3(11, 100, 20), glm::max(pos, glm::vec3(-11, -100, 1)));
			entity.Position(pos);

			// ボタンが押されたら自機から弾を発射する
			if (gamepad.buttons & GamePad::A) {
				shotInterval -= delta;// shotIntervalが減算

				// shotIntervalが0以下になったら自機の左右から1発ずつ、合計2発の弾が発射
				if (shotInterval <= 0) {
					glm::vec3 pos = entity.Position();
					pos.x -= 0.3f;
					for (int i = 0; i < 2; ++i) {
						// 「NormalShot」という名前のモデルはPlayer.fbxファイルに含まれている
						if (Entity::Entity* p = game.AddEntity(EntityGroupId_PlayerShot, pos,
							"NormalShot", "Res/Player.bmp", UpdatePlayerShot())) {
							p->Velocity(glm::vec3(0, 0, 80));
							p->Collision(collisionDataList[EntityGroupId_PlayerShot]);
						}
						pos.x += 0.6f;
					}
					// 0.25秒後に次に弾が発射されるように設定(この値を小さくすれば連射力が高まる)
					shotInterval = 0.25;
					// 音声再生.
					game.PlayAudio(0, CRI_SAMPLECUESHEET_PLAYERSHOT);
				}
			}
			else {
				// ボタンが押されていなかったなら
				shotInterval = 0;// 次にボタンが押された時に即座に弾が発射できるように0に設定
			}
		}
	private:
		double shotInterval = 0;
	};

	/**
	* 自機の弾と敵の衝突処理.
	*/
	void PlayerShotAndEnemyCollisionHandler(Entity::Entity& lhs, Entity::Entity& rhs) {
		// 敵機の位置に爆発エンティティを発生させる
		GameEngine& game = GameEngine::Instance();
		if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, rhs.Position(),
			"Blast", "Res/Toroid.bmp", UpdateBlast())) {
			const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
			p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));

			// 得点を加算する(敵を撃墜したら100点ずつ加算).
			game.UserVariable("score") += 100;
		}
		// 音声再生.
		game.PlayAudio(1, CRI_SAMPLECUESHEET_BOMB);
		// 削除
		lhs.Destroy();// 自機の弾
		rhs.Destroy();// 敵機
	}

	/**
	* Uniform Block Objectを作成する.
	*
	* @param size Uniform Blockのサイズ.
	* @param data Uniform Blockに転送するデータへのポインタ.
	*
	* @return 作成したUBO.
	*/
	GLuint CreateUBO(GLsizeiptr size, const GLvoid* data = nullptr){
		GLuint ubo;
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		return ubo;
	}

	// コンストラクタ.
	MainGame::MainGame(Entity::Entity* p) : pSpaceSphere(p) {

		GameEngine& game = GameEngine::Instance();

		// 得点を初期化
		//「score」という名前のグローバル変数を0で初期化
		GameEngine::Instance().UserVariable("score") = 0;

		// GameEngineに登録
		game.CollisionHandler(EntityGroupId_PlayerShot, EntityGroupId_Enemy,
			&PlayerShotAndEnemyCollisionHandler);
	}

	// デストラクタ.
	MainGame::~MainGame() {
		/// Do Nothing.
	}

	/// タイトル画面の更新.
	void MainGame::operator()(double delta){
		GameEngine& game = GameEngine::Instance();

		// 自機作成コード
		if (!pPlayer) {
			pPlayer = game.AddEntity(EntityGroupId_Player, glm::vec3(0, 0, 2),
				"Aircraft", "Res/Player.bmp", UpdatePlayer());
			pPlayer->Collision(collisionDataList[EntityGroupId_Player]);//自機の衝突形状
		}

		game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });
		game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
		game.Light(0, { glm::vec4(40, 100, 10, 1), glm::vec4(12000, 12000, 12000, 1) });
		std::uniform_int_distribution<> distributerX(-12, 12);
		std::uniform_int_distribution<> distributerZ(40, 44);
		interval -= delta;
		if (interval <= 0) {
			const std::uniform_real_distribution<> rndInterval(2.0, 5.0);//出現頻度
			const std::uniform_int_distribution<> rndAddingCount(1, 5);
			for (int i = rndAddingCount(game.Rand()); i > 0; --i) {
				const glm::vec3 pos(distributerX(game.Rand()), 0, distributerZ(game.Rand()));
				//敵機を作成
				if (Entity::Entity* p = game.AddEntity(EntityGroupId_Enemy, pos,
					"Toroid", "Res/Toroid.bmp", UpdateToroid())) {
					p->Velocity(glm::vec3(pos.x < 0 ? 4.0f : -4.0f, 0, -16));//移動速度
					p->Collision(collisionDataList[EntityGroupId_Enemy]);//敵機の衝突形状
				}
			}
			interval = rndInterval(game.Rand());
		}
		// 得点を表示する.
		char str[16];
		snprintf(str, 16, "%08.0f", game.UserVariable("score"));
		game.AddString(glm::vec2(-0.2f, 1.0f), str);
	}
}