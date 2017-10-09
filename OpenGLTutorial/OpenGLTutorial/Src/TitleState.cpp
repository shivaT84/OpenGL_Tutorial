/**
* @file TitleState.cpp
*/

#include "GameState.h"
#include "GameEngine.h"
#include "../Res/Audio/SampleCueSheet.h"

namespace GameState {

	/// �w�i�̍X�V.
	void UpdateSpaceSphere(Entity::Entity& entity, double delta){

		// �w�i���������Ɖ�]������.
		glm::vec3 rotSpace = glm::eulerAngles(entity.Rotation());
		rotSpace.x += static_cast<float>(glm::radians(2.5) * delta);
		entity.Rotation(rotSpace);
	}

	/// �^�C�g����ʂ̍X�V.
	void Title::operator()(double delta){

		GameEngine& game = GameEngine::Instance();
		game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });

		if (!pSpaceSphere) {
			// �w�i�G���e�B�e�B���쐬.
			pSpaceSphere = game.AddEntity(EntityGroupId_Others, glm::vec3(0, 0, 0),
				"SpaceSphere", "Res/SpaceSphere.bmp", &UpdateSpaceSphere, "NonLighting");
		}

		const float offset = timer == 0 ? 0 : (2.0f - timer) * (2.0f - timer) * 2.0f;
		game.FontScale(glm::vec2(2));
		game.AddString(glm::vec2(-0.25f + offset, 0.125f), "STAR FIGHTER");
		game.FontScale(glm::vec2(0.5f));
		game.AddString(glm::vec2(0.1f + offset, -0.05f), "The ultimate in Manned-Fighter");
		if (timer > 0) {
			timer -= static_cast<float>(delta);
			if (timer <= 0) {
				game.UpdateFunc(MainGame(pSpaceSphere));
			}
		}else if (game.GetGamePad().buttonDown & GamePad::START) {
			game.PlayAudio(1, CRI_SAMPLECUESHEET_START);
			timer = 2;
		}
	}
}