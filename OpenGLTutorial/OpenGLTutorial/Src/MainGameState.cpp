/**
* @file MainGameState.cpp
*/
#include "GameState.h"
#include "GameEngine.h"
#include "../Res/Audio/SampleCueSheet.h"
#include <algorithm>

namespace GameState {

	/// �Փˌ`�󃊃X�g.
	static const Entity::CollisionData collisionDataList[] = {
		{},//�w�i�p�̃_�~�[�f�[�^.
		{ glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
		{ glm::vec3(-0.5f, -0.5f, -1.0f), glm::vec3(0.5f, 0.5f, 1.0f) },
		{ glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
		{ glm::vec3(-0.25f, -0.25f, -0.25f), glm::vec3(0.25f, 0.25f, 0.25f) },
	};

	/**
	* �G�̉~�Ղ̏�Ԃ��X�V����.
	*/
	struct UpdateToroid {

		void operator()(Entity::Entity& entity, double delta) {
			// �͈͊O�ɏo����폜����.
			const glm::vec3 pos = entity.Position();
			if (std::abs(pos.x) > 40.0f || std::abs(pos.z) > 40.0f) {
				GameEngine::Instance().RemoveEntity(&entity);
				return;
			}

			// �~�Ղ���]������.
			float rot = glm::angle(entity.Rotation());
			rot += glm::radians(15.0f) * static_cast<float>(delta);
			if (rot > glm::pi<float>() * 2.0f) {
				rot -= glm::pi<float>() * 2.0f;
			}
			entity.Rotation(glm::angleAxis(rot, glm::vec3(0, 1, 0)));
		}
	};

	/**
	* ���@�̒e�̍X�V.
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
	* @desc	�����̍X�V.
	* @tips	���Ԍo�߂ɉ����đ傫���A�F�A�p�x��ω������Ă䂫�A��莞�Ԃ��o�߂�����폜����
	*/
	struct UpdateBlast {
		void operator()(Entity::Entity& entity, double delta) {
			// ��莞�Ԃ��o�߂�����폜
			timer += delta;
			if (timer >= 0.5) {
				entity.Destroy();
				return;
			}
			const float variation = static_cast<float>(timer * 4); // �ω���.
			// �傫����1�{����X�^�[�g���Đ��`�Ɋg�嗦���グ�Ă����A���Œ��O�ɂ��悻3�{�ɂȂ�悤�ɐݒ�
			entity.Scale(glm::vec3(static_cast<float>(1 + variation))); // ���X�Ɋg�傷��.

			// ���Ԍo�߂ŐF�Ɠ����x��ω�������.
			static const glm::vec4 color[] = {
				glm::vec4(1.0f, 1.0f, 0.75f, 1),
				glm::vec4(1.0f, 0.5f, 0.1f, 1),
				glm::vec4(0.25f, 0.1f, 0.1f, 0),
			};
			const glm::vec4 col0 = color[static_cast<int>(variation)];
			const glm::vec4 col1 = color[static_cast<int>(variation) + 1];
			const glm::vec4 newColor = glm::mix(col0, col1, std::fmod(variation, 1));
			entity.Color(newColor);

			// Y���܂���b��60�x�̑��x�ŉ�]������.
			glm::vec3 euler = glm::eulerAngles(entity.Rotation());
			euler.y += glm::radians(60.0f) * static_cast<float>(delta);
			entity.Rotation(glm::quat(euler));
		}

		double timer = 0;
	};

	/**
	* ���@�̍X�V
	*/
	struct UpdatePlayer {
		void operator()(Entity::Entity& entity, double delta) {

			// perator()�̍��W���X�V���鏈��
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
				vec = glm::normalize(vec) * 15.0f;//�ړ����x
			}
			entity.Velocity(vec);
			entity.Rotation(glm::quat(glm::vec3(0, 0, rotZ)));

			glm::vec3 pos = entity.Position();
			pos = glm::min(glm::vec3(11, 100, 20), glm::max(pos, glm::vec3(-11, -100, 1)));
			entity.Position(pos);

			// �{�^���������ꂽ�玩�@����e�𔭎˂���
			if (gamepad.buttons & GamePad::A) {
				shotInterval -= delta;// shotInterval�����Z

				// shotInterval��0�ȉ��ɂȂ����玩�@�̍��E����1�����A���v2���̒e������
				if (shotInterval <= 0) {
					glm::vec3 pos = entity.Position();
					pos.x -= 0.3f;
					for (int i = 0; i < 2; ++i) {
						// �uNormalShot�v�Ƃ������O�̃��f����Player.fbx�t�@�C���Ɋ܂܂�Ă���
						if (Entity::Entity* p = game.AddEntity(EntityGroupId_PlayerShot, pos,
							"NormalShot", "Res/Player.bmp", UpdatePlayerShot())) {
							p->Velocity(glm::vec3(0, 0, 80));
							p->Collision(collisionDataList[EntityGroupId_PlayerShot]);
						}
						pos.x += 0.6f;
					}
					// 0.25�b��Ɏ��ɒe�����˂����悤�ɐݒ�(���̒l������������ΘA�˗͂����܂�)
					shotInterval = 0.25;
					// �����Đ�.
					game.PlayAudio(0, CRI_SAMPLECUESHEET_PLAYERSHOT);
				}
			}
			else {
				// �{�^����������Ă��Ȃ������Ȃ�
				shotInterval = 0;// ���Ƀ{�^���������ꂽ���ɑ����ɒe�����˂ł���悤��0�ɐݒ�
			}
		}
	private:
		double shotInterval = 0;
	};

	/**
	* ���@�̒e�ƓG�̏Փˏ���.
	*/
	void PlayerShotAndEnemyCollisionHandler(Entity::Entity& lhs, Entity::Entity& rhs) {
		// �G�@�̈ʒu�ɔ����G���e�B�e�B�𔭐�������
		GameEngine& game = GameEngine::Instance();
		if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, rhs.Position(),
			"Blast", "Res/Toroid.bmp", UpdateBlast())) {
			const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
			p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));

			// ���_�����Z����(�G�����Ă�����100�_�����Z).
			game.UserVariable("score") += 100;
		}
		// �����Đ�.
		game.PlayAudio(1, CRI_SAMPLECUESHEET_BOMB);
		// �폜
		lhs.Destroy();// ���@�̒e
		rhs.Destroy();// �G�@
	}

	/**
	* Uniform Block Object���쐬����.
	*
	* @param size Uniform Block�̃T�C�Y.
	* @param data Uniform Block�ɓ]������f�[�^�ւ̃|�C���^.
	*
	* @return �쐬����UBO.
	*/
	GLuint CreateUBO(GLsizeiptr size, const GLvoid* data = nullptr){
		GLuint ubo;
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		return ubo;
	}

	// �R���X�g���N�^.
	MainGame::MainGame(Entity::Entity* p) : pSpaceSphere(p) {

		GameEngine& game = GameEngine::Instance();

		// ���_��������
		//�uscore�v�Ƃ������O�̃O���[�o���ϐ���0�ŏ�����
		GameEngine::Instance().UserVariable("score") = 0;

		// GameEngine�ɓo�^
		game.CollisionHandler(EntityGroupId_PlayerShot, EntityGroupId_Enemy,
			&PlayerShotAndEnemyCollisionHandler);
	}

	// �f�X�g���N�^.
	MainGame::~MainGame() {
		/// Do Nothing.
	}

	/// �^�C�g����ʂ̍X�V.
	void MainGame::operator()(double delta){
		GameEngine& game = GameEngine::Instance();

		// ���@�쐬�R�[�h
		if (!pPlayer) {
			pPlayer = game.AddEntity(EntityGroupId_Player, glm::vec3(0, 0, 2),
				"Aircraft", "Res/Player.bmp", UpdatePlayer());
			pPlayer->Collision(collisionDataList[EntityGroupId_Player]);//���@�̏Փˌ`��
		}

		game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });
		game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
		game.Light(0, { glm::vec4(40, 100, 10, 1), glm::vec4(12000, 12000, 12000, 1) });
		std::uniform_int_distribution<> distributerX(-12, 12);
		std::uniform_int_distribution<> distributerZ(40, 44);
		interval -= delta;
		if (interval <= 0) {
			const std::uniform_real_distribution<> rndInterval(2.0, 5.0);//�o���p�x
			const std::uniform_int_distribution<> rndAddingCount(1, 5);
			for (int i = rndAddingCount(game.Rand()); i > 0; --i) {
				const glm::vec3 pos(distributerX(game.Rand()), 0, distributerZ(game.Rand()));
				//�G�@���쐬
				if (Entity::Entity* p = game.AddEntity(EntityGroupId_Enemy, pos,
					"Toroid", "Res/Toroid.bmp", UpdateToroid())) {
					p->Velocity(glm::vec3(pos.x < 0 ? 4.0f : -4.0f, 0, -16));//�ړ����x
					p->Collision(collisionDataList[EntityGroupId_Enemy]);//�G�@�̏Փˌ`��
				}
			}
			interval = rndInterval(game.Rand());
		}
		// ���_��\������.
		char str[16];
		snprintf(str, 16, "%08.0f", game.UserVariable("score"));
		game.AddString(glm::vec2(-0.2f, 1.0f), str);
	}
}