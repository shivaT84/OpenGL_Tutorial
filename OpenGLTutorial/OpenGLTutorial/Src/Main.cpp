/**
* @file	Main.cpp
*/

#include "GLFWEW.h"
#include "Texture.h"
#include "Shader.h"
#include "OffscreenBuffer.h"
#include "UniformBuffer.h"
#include "Mesh.h"
#include "Entity.h"
#include <random>
#include <time.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream> 
#include <vector>


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
	{ {-0.5f,-0.3f, 0.5f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
	{ { 0.3f,-0.3f, 0.5f },{ 0.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 0.3f, 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
	{ {-0.5f, 0.5f, 0.5f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },

	{ {-0.3f, 0.3f, 0.1f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
	{ {-0.3f,-0.5f, 0.1f },{ 0.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
	{ { 0.5f,-0.5f, 0.1f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 0.5f,-0.5f, 0.1f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, 0.3f, 0.1f },{ 1.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
	{ {-0.3f, 0.3f, 0.1f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },

	{ {-1.0f,-1.0f, 0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
	{ { 1.0f,-1.0f, 0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
	{ { 1.0f, 1.0f, 0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
	{ {-1.0f, 1.0f, 0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
};

/// インデックスデータ.
const GLuint indices[] = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 7, 8, 9,
	10, 11, 12, 12, 13, 10,
};

/// 頂点シェーダのパラメータ型.
struct VertexData {
	glm::mat4 matMVP;
	glm::mat4 matModel;
	glm::mat3x4 matNormal;
};

/**
* ライトデータ(点光源).
*/
struct PointLight {
	glm::vec4 position; ///< 座標(ワールド座標系).
	glm::vec4 color; ///< 明るさ.
}; 

const int maxLightCount = 4; ///< ライトの数. 

/**
* ライティングパラメータ.
*/
struct LightData {
	glm::vec4 ambientColor; ///< 環境光.
	PointLight light[maxLightCount]; ///< ライトのリスト.
};

/** *
ポストエフェクトデータ.
*/ struct PostEffectData {
	glm::mat4x4 matColor; ///< 色変換行列.
};

/**
* 部分描画データ.
*/
struct RenderingPart {
	GLsizei size; ///< 描画するインデックス数.
	GLvoid* offset; ///< 描画開始インデックスのバイトオフセット.
};

/**
* RenderingPartを作成する.
*
* @param size 描画するインデックス数.
* @param offset 描画開始インデックスのオフセット(インデックス単位).
*
* @return 作成した部分描画オブジェクト. 
*/
constexpr RenderingPart MakeRenderingPart(GLsizei size, GLsizei offset) {
	return{ size, reinterpret_cast<GLvoid*>(offset * sizeof(GLuint)) };
}

/**
* 部分描画データリスト.
*/
static const RenderingPart renderingParts[] = {
	MakeRenderingPart(12, 0),
	MakeRenderingPart(6, 12),
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
GLuint CreateIBO(GLsizeiptr size, const GLvoid* data){
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
* 敵の円盤の状態を更新する.
*/
struct UpdateToroid {
	explicit UpdateToroid(Entity::Buffer& buffer) : entityBuffer(buffer) {}

	void operator()(Entity::Entity& entity, void* ubo, double delta, const glm::mat4& matView, const glm::mat4& matProj) {
		// 範囲外に出たら削除する.
		const glm::vec3 pos = entity.Position();
		if (std::abs(pos.x) > 40.0f || std::abs(pos.z) > 40.0f) {
			entityBuffer.RemoveEntity(&entity);
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
		VertexData data;
		data.matModel = entity.TRSMatrix();
		data.matNormal = glm::mat4_cast(entity.Rotation());
		data.matMVP = matProj * matView * data.matModel;
		memcpy(ubo, &data, sizeof(VertexData));
	} 

	Entity::Buffer& entityBuffer;
};

/**
* ゲームの状態を更新する.
*
* @param entityBuffer 敵エンティティ追加先のエンティティバッファ.
* @param meshBuffer   敵エンティティのメッシュを管理しているメッシュバッファ.
* @param tex          敵エンティティ用のテクスチャ.
* @param prog         敵エンティティ用のシェーダープログラム.
*/
void Update(Entity::BufferPtr entityBuffer, Mesh::BufferPtr meshBuffer, TexturePtr tex, Shader::ProgramPtr prog) {
	static std::mt19937 rand(time(nullptr));
	static double interval = 0;

	interval -= 1.0 / 60.0;
	if (interval <= 0) {
		const std::uniform_real_distribution<float> posXRange(-15, 15);
		const glm::vec3 pos(posXRange(rand), 0, 40);
		const Mesh::MeshPtr& mesh = meshBuffer->GetMesh("Toroid");
		if (Entity::Entity* p = entityBuffer->AddEntity(pos, mesh, tex, prog, UpdateToroid(*entityBuffer))) {
			p->Velocity(glm::vec3(pos.x < 0 ? 0.1f : -0.1f, 0, -1.0f));
		}
		const std::uniform_real_distribution<double> intervalRange(3.0, 6.0);
		interval = intervalRange(rand);
	}
}

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

/**
* GLFW からのエラー報告を処理する.
*
* @param error エラー番号.
* @param desc  エラーの内容.
*/
void ErrorCallback(int error, const char* desc) {
	std::cerr << "ERROR: " << desc << std::endl;
}

/// エントリーポイント
int main() {

	// エラーコールバック
	glfwSetErrorCallback(ErrorCallback);

	// GLFWEWの初期化
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	if (!window.Init(800, 600, "OpenGL Tutorial")) {
		return 1;
	}

	// 描画デバイス、対応バージョン出力
	const GLubyte* renderer = glGetString(GL_RENDERER);
	std::cout << "Renderer: " << renderer << std::endl;
	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "Version: " << version << std::endl;

	// オブジェクトの作成
	const GLuint vbo = CreateVBO(sizeof(vertices), vertices);
	const GLuint ibo = CreateIBO(sizeof(indices), indices);
	const GLuint vao = CreateVAO(vbo, ibo);
	const UniformBufferPtr uboVertex = UniformBuffer::Create(sizeof(VertexData), 0, "VertexData");
	// ライトデータ用の UBO を作成
	const UniformBufferPtr uboLight = UniformBuffer::Create(sizeof(LightData), 1, "LightData");
	// 色変換行列用のUBOを作成
	const UniformBufferPtr uboPostEffect = UniformBuffer::Create(sizeof(PostEffectData), 2, "PostEffectData");
	
	const GLuint ubo = CreateUBO(sizeof(VertexData));

	// シェーダーを読み込む
	// チュートリアルシェーダー
	const Shader::ProgramPtr progTutorial = Shader::Program::Create("Res/Tutorial.vert", "Res/Tutorial.frag");
	// カラーフィルターシェーダー
	//const Shader::ProgramPtr progColorFilter = Shader::Program::Create("Res/ColorFilter.vert", "Res/ColorFilter.frag");
	// ポスター化シェーダー
	const Shader::ProgramPtr progColorFilter = Shader::Program::Create("Res/Posterization.vert", "Res/Posterization.frag");
	
	if (!vbo || !ibo || !vao || !uboVertex || !uboLight || !progTutorial || !progColorFilter) {
		return 1;
	}
	progTutorial->UniformBlockBinding("VertexData", 0);
	progTutorial->UniformBlockBinding("LightData", 1);

	/*
	const GLuint uboIndex = glGetUniformBlockIndex(shaderProgram, "VertexData");
	if (uboIndex == GL_INVALID_INDEX) {
		return 1;
	}
	glUniformBlockBinding(shaderProgram, uboIndex, 0); 
	*/
	// テクスチャデータ.
	static const uint32_t textureData[] = {
		0xffffffff, 0xffcccccc, 0xffffffff, 0xffcccccc, 0xffffffff,
		0xff888888, 0xffffffff, 0xff888888, 0xffffffff, 0xff888888,
		0xffffffff, 0xff444444, 0xffffffff, 0xff444444, 0xffffffff,
		0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000,
		0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff,
	}; 

	//TexturePtr tex = Texture::Create(5, 5, GL_RGBA8, GL_RGBA, textureData);
	TexturePtr tex = Texture::LoadFromFile("Res/Sample.bmp");
	TexturePtr texToroid = Texture::LoadFromFile("Res/Toroid.bmp");
	if (!tex || !texToroid) {
		return 1;
	}

	Mesh::BufferPtr meshBuffer = Mesh::Buffer::Create(10 * 1024, 30 * 1024);
	meshBuffer->LoadMeshFromFile("Res/Toroid.fbx");

	Entity::BufferPtr entityBuffer = Entity::Buffer::Create(1024, sizeof(VertexData), 0, "VertexData");
	if (!entityBuffer) {
		return 1;
	}
	std::mt19937 rand(static_cast<unsigned int>(time(nullptr)));

	// 深度バッファ使用
	glEnable(GL_DEPTH_TEST);

	// オフスクリーンバッファ作成
	const OffscreenBufferPtr offscreen = OffscreenBuffer::Create(800, 600);

	// メインループ
	while (!window.ShouldClose()) {

		Update(entityBuffer, meshBuffer, texToroid, progTutorial);

		// 描画を全て変更したオフスクリーンバッファに対して行われる
		glBindFramebuffer(GL_FRAMEBUFFER, offscreen->GetFramebuffer());

		glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 視点を動かす
		const glm::vec3 viewPos = glm::vec4(0, 20, -20, 1);

		// 頂点データの描画
		progTutorial->UseProgram();

		// UBO にデータを転送
		const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		const glm::mat4x4 matView = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		VertexData vertexData;
		vertexData.matMVP = matProj * matView;
		//vertexData.lightPosition = glm::vec4(1, 1, 1, 1);
		//vertexData.lightColor = glm::vec4(2, 2, 2, 1);
		//vertexData.ambientColor = glm::vec4(0.05f, 0.1f, 0.2f, 1);
		// UBO へデータを転送
		uboVertex->BufferSubData(&vertexData);

		LightData lightData;
		lightData.ambientColor= glm::vec4(0.05f, 0.1f, 0.2f, 1);

		lightData.light[0].color = glm::vec4(12000, 12000, 12000, 1);
		lightData.light[0].position = glm::vec4(40, 100, 20, 1);

		uboLight->BufferSubData(&lightData);

		// 座標変換行列を作成し、uniform変数に転送する
		/*
		const GLint matMVPLoc = glGetUniformLocation(shaderProgram, "matMVP");
		if (matMVPLoc >= 0) {
			const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
			const glm::mat4x4 matView = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			const glm::mat4x4 matMVP = matProj * matView;
			glUniformMatrix4fv(matMVPLoc, 1, GL_FALSE, &matMVP[0][0]);
		}*/

		// テクスチャ設定
		progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, tex->Id());

		glBindVertexArray(vao);

		//インデックスデータによる描画
		glDrawElements(
			GL_TRIANGLES, renderingParts[0].size,
			GL_UNSIGNED_INT, renderingParts[0].offset
		);

		// メッシュの表示
		progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, texToroid->Id());
		meshBuffer->BindVAO();
		meshBuffer->GetMesh("Toroid")->Draw(meshBuffer);

		entityBuffer->Update(1.0 / 60.0, matView, matProj);
		entityBuffer->Draw(meshBuffer);

		glBindVertexArray(vao);

		// オフスクリーンバッファを使ってバックバッファを描画する
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.5f, 0.3f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// モノトーンシェーダー使用
		progColorFilter->UseProgram();
		progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());

		PostEffectData postEffect;
		postEffect.matColor[0] = glm::vec4(0.393f, 0.349f, 0.272f, 0);
		postEffect.matColor[1] = glm::vec4(0.769f, 0.686f, 0.534f, 0);
		postEffect.matColor[2] = glm::vec4(0.189f, 0.168f, 0.131f, 0);
		postEffect.matColor[3] = glm::vec4(0, 0, 0, 1);
		uboPostEffect->BufferSubData(&postEffect);
		
		// 初期化
		//vertexData = {};
		//vertexData.ambientColor = glm::vec4(1, 1, 1, 1);
		// UBO にデータを転送
		//uboVertex->BufferSubData(&vertexData);
		//lightData = {};
		//lightData.ambientColor= glm::vec4(1);
		//uboLight->BufferSubData(&lightData);

		glDrawElements(
			GL_TRIANGLES, renderingParts[1].size,
			GL_UNSIGNED_INT, renderingParts[1].offset
		);

		window.SwapBuffers();
	}

	// オブジェクトの削除
	//glDeleteProgram(shaderProgram);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	// 終了処理
	glfwTerminate();

	return 0;
} 
