/**
* @file GameEngine.cpp
*/
#include "GameEngine.h"
#include "Audio.h"
#include "GLFWEW.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <time.h>


// 頂点データ型の定義
/// 3D ベクター型.
struct Vector3 {
	float x, y, z;
};
/// RGBA カラー型.
struct Color {
	float r, g, b, a;
};

/// 頂点データ型.
struct Vertex {
	Vector3 position; ///< 座標 
	Color color; ///< 色
	glm::vec2 texCoord; ///< テクスチャ座標
};


/// 頂点データ.
const Vertex vertices[] = {
	{ { -0.5f,-0.3f, 0.5f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
	{ { 0.3f,-0.3f, 0.5f },{ 0.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 0.3f, 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
	{ { -0.5f, 0.5f, 0.5f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },

	{ { -0.3f, 0.3f, 0.1f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
	{ { -0.3f,-0.5f, 0.1f },{ 0.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
	{ { 0.5f,-0.5f, 0.1f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 0.5f,-0.5f, 0.1f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, 0.3f, 0.1f },{ 1.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
	{ { -0.3f, 0.3f, 0.1f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },

	{ { -1.0f,-1.0f, 0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
	{ { 1.0f,-1.0f, 0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 1.0f, 1.0f, 0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
	{ { -1.0f, 1.0f, 0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
};

/// インデックスデータ.
const GLuint indices[] = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 7, 8, 9,
	10, 11, 12, 12, 13, 10,
};


/**
* 部分描画データ.
*/
struct RenderingData {
	GLsizei size; ///< 描画するインデックス数.
	GLvoid* offset; ///< 描画開始インデックスのバイトオフセット.
};

/**
* RenderingDataを作成する.
*
* @param size 描画するインデックス数.
* @param offset 描画開始インデックスのオフセット(インデックス単位).
*
* @return 作成した部分描画オブジェクト.
*/
constexpr RenderingData MakeRenderingData(GLsizei size, GLsizei offset) {
	return{ size, reinterpret_cast<GLvoid*>(offset * sizeof(GLuint)) };
}

/**
* 部分描画データリスト.
*/
static const RenderingData renderingData[] = {
	MakeRenderingData(12, 0),
	MakeRenderingData(6, 12),
};

/**
* Vertex Buffer Object を作成する.
*
* @param size 頂点データのサイズ.
* @param data 頂点データへのポインタ.
*
* @return 作成した VBO.
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
* 頂点アトリビュートを設定する
* Index Buffer Objectを作成する
*
* @param size インデックスデータのサイズ
* @param data インデックスデータへのポインタ
*
* @return 作成したIBO
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
* 頂点アトリビュートを設定する.
*
* @param index 頂点アトリビュートのインデックス.
* @param cls   頂点データ型名.
* @param mbr   頂点アトリビュートに設定する cls のメンバ変数名.
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
* Vertex Array Object を作成する.
*
* @param vbo VAO に関連付けられる VBO.
* @param ibo VAO に関連付けられる IBO.
*
* @return 作成した VAO.
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
* ゲームエンジンのインスタンスを取得する.
*
* @return ゲームエンジンのインスタンス.
*/
GameEngine& GameEngine::Instance() {
	static GameEngine instance;
	return instance;
}

/**
* ゲームエンジンを初期化する.
*
* @param w     ウィンドウの描画範囲の幅(ピクセル).
* @param h     ウィンドウの描画範囲の高さ(ピクセル).
* @param title ウィンドウタイトル(UTF-8の0終端文字列).
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*
* Update, Render関数などを呼び出す前に、一度だけ呼び出しておく必要がある.
* 一度初期化に成功すると、以後の呼び出しではなにもせずにtrueを返す.
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

	offscreen = OffscreenBuffer::Create(800, 600);

	if (!vbo || !ibo || !vao || !uboLight || !uboPostEffect || !offscreen) {
		std::cerr << "ERROR: GameEngine の初期化に失敗" << std::endl;
		return false;
	}

	// シェーダ名とシェーダファイル名をセットにしてshaderNameListという配列に格納.
	static const char* const shaderNameList[][3] = {
		{ "Tutorial", "Res/Tutorial.vert", "Res/Tutorial.frag" },
		{ "ColorFilter", "Res/ColorFilter.vert", "Res/ColorFilter.frag" },
		{ "NonLighting", "Res/NonLighting.vert", "Res/NonLighting.frag" },
	};
	shaderMap.reserve(sizeof(shaderNameList) / sizeof(shaderNameList[0]));
	// 全てのシェーダを作成.
	for (auto& e : shaderNameList) {
		Shader::ProgramPtr program = Shader::Program::Create(e[1], e[2]);
		if (!program) {
			return false;
		}
		shaderMap.insert(std::make_pair(std::string(e[0]), program));
	}

	shaderMap["Tutorial"]->UniformBlockBinding("VertexData", 0);
	shaderMap["Tutorial"]->UniformBlockBinding("LightData", 1);
	shaderMap["ColorFilter"]->UniformBlockBinding("PostEffectData", 2);

	meshBuffer = Mesh::Buffer::Create(10 * 1024, 30 * 1024);
	if (!meshBuffer) {
		std::cerr << "ERROR: GameEngine の初期化に失敗" << std::endl;
		return false;
	}
	entityBuffer = Entity::Buffer::Create(1024, sizeof(Uniform::VertexData), 0, "VertexData");
	if (!entityBuffer) {
		std::cerr << "ERROR: GameEngine の初期化に失敗" << std::endl;
		return false;
	}
	rand.seed(std::random_device()());
	// //Font::Rendererを初期化
	fontRenderer.Init(1024, glm::vec2(800, 600));

	isInitialized = true;
	return true;
}

/**
* @desc	ゲームを実行する.
* @tips	開発環境と実行環境で処理速度が異なる場合、
*		ゲームの動作速度が意図したものとは異なる可能性がある。
*		そのため、実際の経過時間を計測する。
*/
void GameEngine::Run(){
	GLFWEW::Window& window = GLFWEW::Window::Instance();

	// 現在の時刻(秒)を取得する(ブレークポイントなどの一時停止中も進む)
	double prevTime = glfwGetTime();

	while (!window.ShouldClose()) {
		const double curTime = glfwGetTime();
		const double delta = curTime - prevTime;
		prevTime = curTime;
		window.UpdateGamePad();
		Update(glm::min(0.25, delta));//デバッグ用に0.25を越えないように制御
		Render();
		window.SwapBuffers();
	}
}

/**
* 状態更新関数を設定する.
*
* @param func 設定する更新関数.
*/
void GameEngine::UpdateFunc(const UpdateFuncType& func){
	updateFunc = func;
}

/**
* 状態更新関数を取得する.
*
* @return 設定されている更新関数.
*/
const GameEngine::UpdateFuncType& GameEngine::UpdateFunc() const{
	return updateFunc;
}

/**
* テクスチャを読み込む.
*
* @param filename テクスチャファイル名.
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
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
* メッシュを読み込む.
*
* @param filename メッシュファイル名.
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
*/
bool GameEngine::LoadMeshFromFile(const char* filename){
	return meshBuffer->LoadMeshFromFile(filename);
}


/// エンティティを追加する.
Entity::Entity* GameEngine::AddEntity(int groupId, const glm::vec3& pos, const char* meshName,
	const char* texName, Entity::Entity::UpdateFuncType func, const char* shader){

	// ノーマルマップテクスチャ名をnullptrにして、オーバーロードバージョンを呼び出す.
	return AddEntity(groupId, pos, meshName, texName, nullptr, func, shader);
}

/**
* エンティティを追加する(オーバーロードバージョン).
*
* @param groupId  エンティティのグループID.
* @param position エンティティの座標.
* @param meshName エンティティの表示に使用するメッシュ名.
* @param texName  エンティティの表示に使うテクスチャファイル名.
* @param func     エンティティの状態を更新する関数(または関数オブジェクト).
* @param shader   エンティティの表示に使うシェーダ名.
*
* @return 追加したエンティティへのポインタ.
*         これ以上エンティティを追加できない場合はnullptrが返される.
*         回転や拡大率はこのポインタ経由で設定する.
*         なお、このポインタをアプリケーション側で保持する必要はない.
*/
Entity::Entity* GameEngine::AddEntity(int groupId, const glm::vec3& pos,
	const char* meshName, const char* texName, const char* normalName,
	Entity::Entity::UpdateFuncType func, const char* shader){

	decltype(shaderMap)::const_iterator itr = shaderMap.end();
	// シェーダ名が指定されているかどうか.
	if (shader) {
		// 指定されている場合はそのシェーダ名にする.
		itr = shaderMap.find(shader);
	}
	// 指定されていない場合はTutorialをシェーダ名にする.
	if (itr == shaderMap.end()) {
		itr = shaderMap.find("Tutorial");
		if (itr == shaderMap.end()) {
			return nullptr;
		}
	}
	// shaderMapから対応するシェーダプログラムを取得
	const Mesh::MeshPtr& mesh = meshBuffer->GetMesh(meshName);
	TexturePtr tex[2];
	tex[0] = GetTexture(texName);
	if (normalName) {//0にすると「Dummy」
		tex[1] = GetTexture(normalName);
	} else {
		tex[1] = GetTexture("Res/Dummy.Normal.bmp");
	}
	return entityBuffer->AddEntity(groupId, pos, mesh, tex, itr->second, func);
}

/**
*　エンティティを削除する.
*
* @param 削除するエンティティのポインタ.
*/
void GameEngine::RemoveEntity(Entity::Entity* e){
	entityBuffer->RemoveEntity(e);
}

/**
* ライトを設定する.
*
* @param indes  設定するライトのインデックス.
* @param light  ライトパラメータ.
*/
void GameEngine::Light(int index, const Uniform::PointLight& light){
	if (index < 0 || index >= Uniform::maxLightCount) {
		std::cerr << "WARNING: '" << index << "'は不正なライトインデックスです" << std::endl;
		return;
	}
	lightData.light[index] = light;
}

/**
* ライトを取得する.
*
* @param index 取得するライトのインデックス.
*
* @return ライトパラメータ.
*/
const Uniform::PointLight& GameEngine::Light(int index) const{
	if (index < 0 || index >= Uniform::maxLightCount) {
		std::cerr << "WARNING: '" << index << "'は不正なライトインデックスです" << std::endl;
		static const Uniform::PointLight dummy;
		return dummy;
	}
	return lightData.light[index];
}

/**
* 環境光を設定する.
*
* @param color 環境光の明るさ.
*/
void GameEngine::AmbientLight(const glm::vec4& color){
	lightData.ambientColor = color;
}

/**
* 環境光を取得する.
*
* @return 環境光の明るさ.
*/
const glm::vec4& GameEngine::AmbientLight() const{
	return lightData.ambientColor;
}

/**
* 視点の位置と姿勢を設定する.
*
* @param cam 設定するカメラデータ.
*/
void GameEngine::Camera(const CameraData& cam){
	camera = cam;
}

/**
* 視点の位置と姿勢を取得する.
*
* @return カメラデータ.
*/
const GameEngine::CameraData& GameEngine::Camera() const{
	return camera;
}

/**
* 乱数オブジェクトを取得する.
*
* @return 乱数オブジェクト.
*/
std::mt19937& GameEngine::Rand(){
	return rand;
}

/**
* ゲームパッドの状態を取得する.
*/
const GamePad& GameEngine::GetGamePad() const {
	return GLFWEW::Window::Instance().GetGamePad();
}

/**
* 衝突解決ハンドラを設定する.
*
* @param gid0    衝突対象のグループID.
* @param gid1    衝突対象のグループID.
* @param handler 衝突解決ハンドラ.
*
* 衝突が発生し衝突解決ハンドラが呼びされるとき、より小さいグループIDを持つエンティティから先に渡される.
* ここで指定したグループIDの順序とは無関係であることに注意すること.
* ex)
*   CollisionHandler(10, 1, Func)
*   というコードでハンドラを登録したとする. 衝突が発生すると、
*   Func(グループID=1のエンティティ、グループID=10のエンティティ)
*   のように呼び出される.
*/
void GameEngine::CollisionHandler(int gid0, int gid1, Entity::CollisionHandlerType handler){
	entityBuffer->CollisionHandler(gid0, gid1, handler);
}

/**
* 衝突解決ハンドラを取得する.
*
* @param gid0 衝突対象のグループID.
* @param gid1 衝突対象のグループID.
*
* @return 衝突解決ハンドラ.
*/
const Entity::CollisionHandlerType& GameEngine::CollisionHandler(int gid0, int gid1) const{
	return entityBuffer->CollisionHandler(gid0, gid1);
}

/**
* 衝突解決ハンドラのリストをクリアする.
*/
void GameEngine::ClearCollisionHandlerList(){
	entityBuffer->ClearCollisionHandlerList();
}

/**
* @copydoc Audio::Initialize
*/
bool GameEngine::InitAudio(const char* acfPath, const char* acbPath,
	const char* awbPath, const char* dspBusName){
	return Audio::Initialize(acfPath, acbPath, awbPath, dspBusName);
}

/**
* @copydoc Audio::Play
*/
void GameEngine::PlayAudio(int playerId, int cueId){
	Audio::Play(playerId, cueId);
}

/**
* @copydoc Audio::Stop
*/
void GameEngine::StopAudio(int playerId){
	Audio::Stop(playerId);
}

/**
* デストラクタ.
*/
GameEngine::~GameEngine(){

	updateFunc = nullptr;
	Audio::Destroy();

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
* ゲームの状態を更新する.
*
* @param delta 前回の更新からの経過時間(秒).
*/
void GameEngine::Update(double delta){

	// 文字用のVBOを準備(更新関数内でAddString関数を呼び出せるようにする).
	fontRenderer.MapBuffer();

	if (updateFunc) {
		updateFunc(delta);
	}

	const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 200.0f);
	const glm::mat4x4 matView = glm::lookAt(camera.position, camera.target, camera.up);
	entityBuffer->Update(delta, matView, matProj);
	// VBOをGPUメモリに転送する.
	fontRenderer.UnmapBuffer();
}

/**
* ゲームの状態を描画する.
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
	const Shader::ProgramPtr& progColorFilter = shaderMap.find("ColorFilter")->second;
	progColorFilter->UseProgram();
	Uniform::PostEffectData postEffect;
	uboPostEffect->BufferSubData(&postEffect);
	progColorFilter->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());
	glDrawElements(GL_TRIANGLES, renderingData[1].size, GL_UNSIGNED_INT, renderingData[1].offset);
	// 文字を描画する.
	fontRenderer.Draw();
}

/**
* GLFW からのエラー報告を処理する.
*
* @param error エラー番号.
* @param desc  エラーの内容.
*/
void ErrorCallback(int error, const char* desc) {
	std::cerr << "ERROR: " << desc << std::endl;
}