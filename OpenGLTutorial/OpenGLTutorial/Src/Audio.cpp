/**
* @file Audio.cpp
*/

#include "Audio.h"
#include <cri_adx2le.h>//ADX2 LEのクラスや関数を使用するため
#include <iostream>
#include <cstdint>

namespace Audio {

	CriAtomExVoicePoolHn voicePool;
	CriAtomDbasId dbas = CRIATOMDBAS_ILLEGAL_ID;
	CriAtomExAcbHn acb;
	CriAtomExPlayerHn player[playerMax];

	/**
	* @desc	オーディオシステム用エラーコールバック.
	* @tips	ADX2 LEシステムからのエラー情報を受け取る.
	*/
	void ErrorCallback(const CriChar8* errid, CriUint32 p1, CriUint32 p2, CriUint32* parray){
		const CriChar8* err = criErr_ConvertIdToMessage(errid, p1, p2);
		std::cerr << err << std::endl;
	}

	/// オーディオシステム用アロケータ.(メモリ確保)
	void* Allocate(void* obj, CriUint32 size) { 
		return new uint8_t[size];
	}

	/// オーディオシステム用デアロケータ.(メモリ解放)
	void Deallocate(void* obj, void* ptr) {
		delete[] static_cast<uint8_t*>(ptr);
	}

	/**
	* オーディオシステムを初期化する.
	*
	* @param acfPath    ACFファイルのパス.
	* @param acbPath    ACBファイルのパス.
	* @param awbPath    AWBファイルのパス.
	* @param dspBusName D-BUS名.
	*
	* @retval true  初期化成功.
	* @retval false 初期化失敗.
	* @tips			ADX2 LEを初期化し、音声を再生する準備をする.
	*/
	bool Initialize(const char* acfPath, const char* acbPath, const char* awbPath, const char* dspBusName){
		// エラーコールバック.
		criErr_SetCallback(ErrorCallback);
		// メモリ管理.
		criAtomEx_SetUserAllocator(Allocate, Deallocate, nullptr);

		CriFsConfig fsConfig;
		// 初期化パラメータを保持する
		CriAtomExConfig_WASAPI libConfig;

		criFs_SetDefaultConfig(&fsConfig);
		criAtomEx_SetDefaultConfig_WASAPI(&libConfig);
		// 並列に読み込みを行うことのできるオブジェクトの総数.
		fsConfig.num_loaders = 64;
		// 読み込むことのできるファイルパスの最大長.
		fsConfig.max_path = 1024;
		libConfig.atom_ex.fs_config = &fsConfig;
		// 同時に制御可能な音声の最大数.
		libConfig.atom_ex.max_virtual_voices = 64;
		// ADX2 LEを初期化
		criAtomEx_Initialize_WASAPI(&libConfig, nullptr, 0);

		// ストリーミング再生を制御するD-BASオブジェクトを作成する.
		dbas = criAtomDbas_Create(nullptr, nullptr, 0);
		// 全体設定を記したACFファイルをライブラリに登録.
		if (criAtomEx_RegisterAcfFile(nullptr, acfPath, nullptr, 0) == CRI_FALSE) {
			return false;
		}
		// D-BASオブジェクトとACFファイルを関連付ける.
		criAtomEx_AttachDspBusSetting(dspBusName, nullptr, 0);

		CriAtomExStandardVoicePoolConfig config;
		criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&config);
		config.num_voices = 16;//同時16音.
		config.player_config.streaming_flag = CRI_TRUE;//ストリーミング再生可能.
		// ADX2はピッチ変更時に音質を維持するため、元の周波数の2倍のメモリを必要とする。
		// そのため、48000の2倍の値にしている.
		config.player_config.max_sampling_rate = 48000 * 2;//最大サンプリングレート
		// 同時発音可能な音声の数と、再生可能な音声の種類を設定.
		voicePool = criAtomExVoicePool_AllocateStandardVoicePool(&config, nullptr, 0);
		// 波形データファイル(ACBファイル及びAWBファイル)を読み込む.
		acb = criAtomExAcb_LoadAcbFile(nullptr, acbPath, nullptr, awbPath, nullptr, 0);
		for (auto& e : player) {
			// 再生制御用のプレイヤーオブジェクトを作成.
			e = criAtomExPlayer_Create(nullptr, nullptr, 0);
		}
		return true;
	}

	/**
	* @desc	オーディオシステムを破棄する.
	* @tips	各オブジェクトに対応する破棄関数を呼び出す.
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
	* オーディオシステムの状態を更新する.
	*/
	void Update(){
		criAtomEx_ExecuteMain();
	}

	/**
	* 音声を再生する.
	*
	* @param playerId 再生に使用するプレイヤーのID.
	* @param cueId    再生するキューのID.
	*/
	void Play(int playerId, int cueId){
		// プレイヤーにキューを割り当て.
		criAtomExPlayer_SetCueId(player[playerId], acb, cueId);
		// 再生を開始.
		criAtomExPlayer_Start(player[playerId]);
	}

	/**
	* 音声を停止する.
	*
	* @param playerId 再生を停止するプレイヤーのID.
	*/
	void Stop(int playerId){
		// 再生を停止.
		criAtomExPlayer_Stop(player[playerId]);
	}
}