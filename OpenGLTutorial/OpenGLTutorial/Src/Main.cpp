/**
* @file	Main.cpp
*/

#include "GameEngine.h"
#include "GameState.h"
#include "../Res/Audio/SampleSound_acf.h"
#include "../Res/Audio/SampleCueSheet.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>


/// �G���g���[�|�C���g
int main() {
	GameEngine& game = GameEngine::Instance();
	if (!game.Init(800, 600, "OpenGL Tutorial")) {
		return 1;
	}

	// �I�[�f�B�I�̏�����.
	if (!game.InitAudio("Res/Audio/SampleSound.acf", "Res/Audio/SampleCueSheet.acb",
		nullptr, CRI_SAMPLESOUND_ACF_DSPSETTING_DSPBUSSETTING_0)) {
		return 1;
	}

	// �t�@�C����ǂݍ���
	game.LoadTextureFromFile("Res/SpaceSphere.bmp");
	game.LoadTextureFromFile("Res/Toroid.bmp");
	game.LoadTextureFromFile("Res/Player.bmp");
	game.LoadMeshFromFile("Res/SpaceSphere.fbx");//���i���f��
	game.LoadMeshFromFile("Res/Toroid.fbx");
	game.LoadMeshFromFile("Res/Player.fbx");
	game.LoadMeshFromFile("Res/Blast.fbx");//�����p�̃��b�V��
	game.LoadFontFromFile("Res/UniNeue.fnt");//�t�H���g�t�@�C��

	game.UpdateFunc(GameState::Title());
	game.Run();
	return 0;
}
