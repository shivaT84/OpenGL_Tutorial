/**
* @file Audio.cpp
*/

#include "Audio.h"
#include <cri_adx2le.h>//ADX2 LE�̃N���X��֐����g�p���邽��
#include <iostream>
#include <cstdint>

namespace Audio {

	CriAtomExVoicePoolHn voicePool;
	CriAtomDbasId dbas = CRIATOMDBAS_ILLEGAL_ID;
	CriAtomExAcbHn acb;
	CriAtomExPlayerHn player[playerMax];

	/**
	* @desc	�I�[�f�B�I�V�X�e���p�G���[�R�[���o�b�N.
	* @tips	ADX2 LE�V�X�e������̃G���[�����󂯎��.
	*/
	void ErrorCallback(const CriChar8* errid, CriUint32 p1, CriUint32 p2, CriUint32* parray){
		const CriChar8* err = criErr_ConvertIdToMessage(errid, p1, p2);
		std::cerr << err << std::endl;
	}

	/// �I�[�f�B�I�V�X�e���p�A���P�[�^.(�������m��)
	void* Allocate(void* obj, CriUint32 size) { 
		return new uint8_t[size];
	}

	/// �I�[�f�B�I�V�X�e���p�f�A���P�[�^.(���������)
	void Deallocate(void* obj, void* ptr) {
		delete[] static_cast<uint8_t*>(ptr);
	}

	/**
	* �I�[�f�B�I�V�X�e��������������.
	*
	* @param acfPath    ACF�t�@�C���̃p�X.
	* @param acbPath    ACB�t�@�C���̃p�X.
	* @param awbPath    AWB�t�@�C���̃p�X.
	* @param dspBusName D-BUS��.
	*
	* @retval true  ����������.
	* @retval false ���������s.
	* @tips			ADX2 LE�����������A�������Đ����鏀��������.
	*/
	bool Initialize(const char* acfPath, const char* acbPath, const char* awbPath, const char* dspBusName){
		// �G���[�R�[���o�b�N.
		criErr_SetCallback(ErrorCallback);
		// �������Ǘ�.
		criAtomEx_SetUserAllocator(Allocate, Deallocate, nullptr);

		CriFsConfig fsConfig;
		// �������p�����[�^��ێ�����
		CriAtomExConfig_WASAPI libConfig;

		criFs_SetDefaultConfig(&fsConfig);
		criAtomEx_SetDefaultConfig_WASAPI(&libConfig);
		// ����ɓǂݍ��݂��s�����Ƃ̂ł���I�u�W�F�N�g�̑���.
		fsConfig.num_loaders = 64;
		// �ǂݍ��ނ��Ƃ̂ł���t�@�C���p�X�̍ő咷.
		fsConfig.max_path = 1024;
		libConfig.atom_ex.fs_config = &fsConfig;
		// �����ɐ���\�ȉ����̍ő吔.
		libConfig.atom_ex.max_virtual_voices = 64;
		// ADX2 LE��������
		criAtomEx_Initialize_WASAPI(&libConfig, nullptr, 0);

		// �X�g���[�~���O�Đ��𐧌䂷��D-BAS�I�u�W�F�N�g���쐬����.
		dbas = criAtomDbas_Create(nullptr, nullptr, 0);
		// �S�̐ݒ���L����ACF�t�@�C�������C�u�����ɓo�^.
		if (criAtomEx_RegisterAcfFile(nullptr, acfPath, nullptr, 0) == CRI_FALSE) {
			return false;
		}
		// D-BAS�I�u�W�F�N�g��ACF�t�@�C�����֘A�t����.
		criAtomEx_AttachDspBusSetting(dspBusName, nullptr, 0);

		CriAtomExStandardVoicePoolConfig config;
		criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&config);
		config.num_voices = 16;//����16��.
		config.player_config.streaming_flag = CRI_TRUE;//�X�g���[�~���O�Đ��\.
		// ADX2�̓s�b�`�ύX���ɉ������ێ����邽�߁A���̎��g����2�{�̃�������K�v�Ƃ���B
		// ���̂��߁A48000��2�{�̒l�ɂ��Ă���.
		config.player_config.max_sampling_rate = 48000 * 2;//�ő�T���v�����O���[�g
		// ���������\�ȉ����̐��ƁA�Đ��\�ȉ����̎�ނ�ݒ�.
		voicePool = criAtomExVoicePool_AllocateStandardVoicePool(&config, nullptr, 0);
		// �g�`�f�[�^�t�@�C��(ACB�t�@�C���y��AWB�t�@�C��)��ǂݍ���.
		acb = criAtomExAcb_LoadAcbFile(nullptr, acbPath, nullptr, awbPath, nullptr, 0);
		for (auto& e : player) {
			// �Đ�����p�̃v���C���[�I�u�W�F�N�g���쐬.
			e = criAtomExPlayer_Create(nullptr, nullptr, 0);
		}
		return true;
	}

	/**
	* @desc	�I�[�f�B�I�V�X�e����j������.
	* @tips	�e�I�u�W�F�N�g�ɑΉ�����j���֐����Ăяo��.
	*/
	void Destroy(){
		for (auto& e : player) {
			if (e) {
				criAtomExPlayer_Destroy(e);
				e = nullptr;
			}
		}
		if (acb) {
			criAtomExAcb_Release(acb);
			acb = nullptr;
		}
		if (voicePool) {
			criAtomExVoicePool_Free(voicePool);
			voicePool = nullptr;
		}
		criAtomEx_UnregisterAcf();
		if (dbas != CRIATOMDBAS_ILLEGAL_ID) {
			criAtomDbas_Destroy(dbas);
			dbas = CRIATOMDBAS_ILLEGAL_ID;
		}
		criAtomEx_Finalize_WASAPI();
	}

	/**
	* �I�[�f�B�I�V�X�e���̏�Ԃ��X�V����.
	*/
	void Update(){
		criAtomEx_ExecuteMain();
	}

	/**
	* �������Đ�����.
	*
	* @param playerId �Đ��Ɏg�p����v���C���[��ID.
	* @param cueId    �Đ�����L���[��ID.
	*/
	void Play(int playerId, int cueId){
		// �v���C���[�ɃL���[�����蓖��.
		criAtomExPlayer_SetCueId(player[playerId], acb, cueId);
		// �Đ����J�n.
		criAtomExPlayer_Start(player[playerId]);
	}

	/**
	* �������~����.
	*
	* @param playerId �Đ����~����v���C���[��ID.
	*/
	void Stop(int playerId){
		// �Đ����~.
		criAtomExPlayer_Stop(player[playerId]);
	}
}