/**
* @file Font.cpp
*/

#include "Font.h"
#include "GameEngine.h"//テクスチャを読み込むため
#include <memory>//std::unique_ptr用
#include <iostream>//デバッグ出力用
#include <stdio.h>//ファイル読み込み

/**
* フォント描画機能を格納する名前空間.
*/
namespace Font {

	/**
	* フォント用頂点データ型.
	*/
	struct Vertex{
		glm::vec2 position;
		glm::u16vec2 uv;
		glm::u8vec4 color;
	};

	/**
	* フォント描画オブジェクトを初期化する.
	*
	* @param maxChar 最大描画文字数.
	* @param screen  描画先スクリーンの大きさ.
	*
	* @retval true  初期化成功.
	* @retval false 初期化失敗.
	*/
	bool Renderer::Init(size_t maxChar, const glm::vec2& screen){
		// 格納可能な最大文字数をRendererクラスが処理できる最大値に制限する
		if (maxChar > (USHRT_MAX + 1) / 4) {
			std::cerr << "WARNING: " << maxChar << "は設定可能な最大文字数を越えています" << std::endl;
			maxChar = (USHRT_MAX + 1) / 4;
		}

		// VBOの作成
		vboCapacity = static_cast<GLsizei>(4 * maxChar);
		vbo.Init(GL_ARRAY_BUFFER, sizeof(Vertex) * vboCapacity, nullptr, GL_STREAM_DRAW);
		{
			std::vector<GLushort> tmp;
			tmp.resize(maxChar * 6);
			GLushort* p = tmp.data();
			for (GLushort i = 0; i < maxChar * 4; i += 4) {
				for (GLushort n : { 0, 1, 2, 2, 3, 0 }) {
					*(p++) = i + n;
				}
			}

			// IBOの作成
			ibo.Init(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6 * maxChar, tmp.data(), GL_STATIC_DRAW);
		}

		// VAOを初期化, 作成
		vao.Init(vbo.Id(), ibo.Id());
		vao.Bind();
		// 3つの頂点アトリビュートを設定
		vao.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, position));
		vao.VertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(Vertex), offsetof(Vertex, uv));
		vao.VertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), offsetof(Vertex, color));
		vao.Unbind();

		// フォント描画用のシェーダの読み込み
		progFont = Shader::Program::Create("Res/Font.vert", "Res/Font.frag");
		if (!progFont) {
			return false;
		}

		// スクリーンの大きさをの逆数をreciprocalScreenSize変数に保存
		reciprocalScreenSize = 2.0f / screen;//スクリーンの座標は-1～+1の範囲を取るため2.0fとする.
		return true;
	}

	/**
	* フォントファイルを読み込む.
	*
	* @param filename フォントファイル名.
	*
	* @retval true  読み込み成功.
	* @retval false 読み込み失敗.
	*/
	bool Renderer::LoadFromFile(const char* filename){

		// std::unique_ptrにファイルポインタを格納
		// fclose関数でファイルポインタの解放
		const std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(filename, "r"), fclose);
		if (!fp) {
			std::cerr << "ERROR: '" << filename << "'のオープンに失敗" << std::endl;
			return false;
		}

		// fscanf関数で1行ずつ読み込む
		int line = 1; ///< フォントファイルの処理中の行.
		int ret = fscanf(fp.get(),
			"info face=%*s size=%*d bold=%*d italic=%*d charset=%*s unicode=%*d stretchH=%*d smooth=%*d aa=%*d padding=%*d,%*d,%*d,%*d spacing=%*d,%*d");
		++line;

		// 直前の改行コードをスキップするために書式文字列の先頭に1文字の空白を付け加えている.
		// この空白がないと正しく読み込むことができない.
		glm::vec2 scale;
		ret = fscanf(fp.get(),
			" common lineHeight=%*d base=%*d scaleW=%f scaleH=%f pages=%*d packed=%*d",
			&scale.x, &scale.y);
		if (ret < 2) {
			std::cerr << "ERROR: '" << filename << "'の読み込みに失敗(line=" << line <<
				")" << std::endl;
			return false;
		}
		const glm::vec2 reciprocalScale(1.0f / scale);
		++line;

		char tex[128];
		ret = fscanf(fp.get(), " page id=%*d file=%127s", tex);
		if (ret < 1) {
			std::cerr << "ERROR: '" << filename << "'の読み込みに失敗(line=" << line <<
				")" << std::endl;
			return false;
		}
		texFilename.assign(tex + 1, tex + strlen(tex) - 1);
		++line;

		int charCount;
		ret = fscanf(fp.get(), " chars count=%d", &charCount);
		if (ret < 1) {
			std::cerr << "ERROR: '" << filename << "'の読み込みに失敗(line=" << line <<
				")" << std::endl;
			return false;
		}
		++line;

		fontList.resize(128);
		for (int i = 0; i < charCount; ++i) {
			FontInfo font;
			glm::vec2 uv;
			ret = fscanf(fp.get(),
				" char id=%d x=%f y=%f width=%f height=%f xoffset=%f yoffset=%f xadvance=%f"
				" page=%*d chnl=%*d", &font.id, &uv.x, &uv.y, &font.size.x, &font.size.y,
				&font.offset.x, &font.offset.y, &font.xadvance);
			if (ret < 8) {
				std::cerr << "ERROR: '" << filename << "'の読み込みに失敗(line=" << line <<
					")" << std::endl;
				return false;
			}
			font.offset.y *= -1;
			uv.y = scale.y - uv.y - font.size.y;
			font.uv[0] = uv * reciprocalScale * 65535.0f;
			font.uv[1] = (uv + font.size) * reciprocalScale * 65535.0f;
			if (font.id < 128) {
				fontList[font.id] = font;
			}
			++line;
		}

		// テクスチャを読み込む.
		if (!GameEngine::Instance().LoadTextureFromFile(texFilename.c_str())) {
			return false;
		}
		return true;
	}

	/**
	* @desc		文字色を設定する.
	*
	* @param	c文字色.
	* @tips		引数で渡された色をglm::clamp関数で0～1の範囲に丸め、
	*			255倍することで0～255の範囲を生成.
	*/
	void Renderer::Color(const glm::vec4& c){
		color = glm::clamp(c, 0.0f, 1.0f) * 255.0f;
	}

	/**
	* 文字列を追加する.
	*
	* @param position 表示開始座標.
	* @param str      追加する文字列.
	*
	* @retval true  追加成功.
	* @retval false 追加失敗.
	* @tips			文字ごとに位置情報を取得し、VBOに4つの頂点データを追加する
	*/
	bool Renderer::AddString(const glm::vec2& position, const char* str){
		Vertex* p = pVBO + vboSize;
		glm::vec2 pos = position;
		for (const char* itr = str; *itr; ++itr) {
			// VBOに頂点データを追加するための空きがない場合は、その時点で追加を打ち切る.
			if (vboSize + 4 > vboCapacity) {
				break;
			}
			const FontInfo& font = fontList[*itr];

			// idが0未満の場合、位置情報がないことを示しているため描画しない.
			// また、文字のXとYいずれかのサイズが0の場合も表示できないため描画しない.
			if (font.id >= 0 && font.size.x && font.size.y) {
				// データをVBOに追加
				const glm::vec2 size = font.size * reciprocalScreenSize  * scale;
				const glm::vec2 offsetedPos = pos + font.offset * reciprocalScreenSize * scale;
				p[0].position = offsetedPos + glm::vec2(0, -size.y);
				p[0].uv = font.uv[0];
				p[0].color = color;

				p[1].position = offsetedPos + glm::vec2(size.x, -size.y);
				p[1].uv = glm::u16vec2(font.uv[1].x, font.uv[0].y);
				p[1].color = color;

				p[2].position = offsetedPos + glm::vec2(size.x, 0);
				p[2].uv = font.uv[1];
				p[2].color = color;

				p[3].position = offsetedPos;
				p[3].uv = glm::u16vec2(font.uv[0].x, font.uv[1].y);
				p[3].color = color;

				p += 4;
				vboSize += 4;
			}
			pos.x += font.xadvance * reciprocalScreenSize.x * scale.x;
		}
		return true;
	}

	/**
	* @desc	VBOをシステムメモリにマッピングする.
	* @tips	AddString関数で頂点データの格納先となるVBOを準備する.
	*/
	void Renderer::MapBuffer(){
		// VBOへのポインタが取得済みかどうか
		if (pVBO) {
			return;
		}
		glBindBuffer(GL_ARRAY_BUFFER, vbo.Id());
		// glMapBufferRange関数でバッファオブジェクトへのポインタを取得して格納.
		// 割り当て範囲をバッファ全体とし、フラグにGL_MAP_INVALIDATE_BUFFER_BITを渡すことで新しいバッファを作成し、
		// 古いバッファをオルファン化する.
		pVBO = static_cast<Vertex*>(glMapBufferRange(GL_ARRAY_BUFFER, 0,
			sizeof(Vertex) * vboCapacity, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// バッファの使用量をリセット.
		vboSize = 0;
	}

	/**
	* @desc	VBOのマッピングを解除する.
	* @tips	MapBufferで取得したバッファオブジェクトへのポインタをOepnGLシステムに返却することで、
	*		頂点データをGPUメモリへ転送するように指示する.
	*/
	void Renderer::UnmapBuffer(){
		// VBOを作成していない、あるいは1文字も追加されていない場合は何もせずに終了.
		if (!pVBO || vboSize == 0) {
			return;
		}
		glBindBuffer(GL_ARRAY_BUFFER, vbo.Id());
		// VBOの割り当てを解除
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		pVBO = nullptr;
	}

	/**
	* フォントを描画する.
	*/
	void Renderer::Draw() const{
		// 文字が1文字も書かれていなければ、描画の必要はないので何もせずに終了.
		if (vboSize == 0) {
			return;
		}
		// OpenGLシステムにフォント用のVAOを割り当て.
		vao.Bind();
		// すべての文字は同じZ座標を持つため、上書きできるように深度テストを無効化.
		glDisable(GL_DEPTH_TEST);
		// 文字には透明な部分があるので、アルファブレンドを有効.
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// フォント用シェーダープログラムを割り当て.
		progFont->UseProgram();
		// テクスチャをバインド.
		progFont->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D,
			GameEngine::Instance().GetTexture(texFilename.c_str())->Id());
		// 全ての文字を描画.
		glDrawElements(GL_TRIANGLES, (vboSize / 4) * 6, GL_UNSIGNED_SHORT, 0);
		vao.Unbind();
	}
}