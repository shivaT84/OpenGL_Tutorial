/**
* @file Font.cpp
*/

#include "Font.h"
#include "GameEngine.h"//�e�N�X�`����ǂݍ��ނ���
#include <memory>//std::unique_ptr�p
#include <iostream>//�f�o�b�O�o�͗p
#include <stdio.h>//�t�@�C���ǂݍ���

/**
* �t�H���g�`��@�\���i�[���閼�O���.
*/
namespace Font {

	/**
	* �t�H���g�p���_�f�[�^�^.
	*/
	struct Vertex{
		glm::vec2 position;
		glm::u16vec2 uv;
		glm::u8vec4 color;
	};

	/**
	* �t�H���g�`��I�u�W�F�N�g������������.
	*
	* @param maxChar �ő�`�敶����.
	* @param screen  �`���X�N���[���̑傫��.
	*
	* @retval true  ����������.
	* @retval false ���������s.
	*/
	bool Renderer::Init(size_t maxChar, const glm::vec2& screen){
		// �i�[�\�ȍő啶������Renderer�N���X�������ł���ő�l�ɐ�������
		if (maxChar > (USHRT_MAX + 1) / 4) {
			std::cerr << "WARNING: " << maxChar << "�͐ݒ�\�ȍő啶�������z���Ă��܂�" << std::endl;
			maxChar = (USHRT_MAX + 1) / 4;
		}

		// VBO�̍쐬
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

			// IBO�̍쐬
			ibo.Init(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6 * maxChar, tmp.data(), GL_STATIC_DRAW);
		}

		// VAO��������, �쐬
		vao.Init(vbo.Id(), ibo.Id());
		vao.Bind();
		// 3�̒��_�A�g���r���[�g��ݒ�
		vao.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, position));
		vao.VertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(Vertex), offsetof(Vertex, uv));
		vao.VertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), offsetof(Vertex, color));
		vao.Unbind();

		// �t�H���g�`��p�̃V�F�[�_�̓ǂݍ���
		progFont = Shader::Program::Create("Res/Font.vert", "Res/Font.frag");
		if (!progFont) {
			return false;
		}

		// �X�N���[���̑傫�����̋t����reciprocalScreenSize�ϐ��ɕۑ�
		reciprocalScreenSize = 2.0f / screen;//�X�N���[���̍��W��-1�`+1�͈̔͂���邽��2.0f�Ƃ���.
		return true;
	}

	/**
	* �t�H���g�t�@�C����ǂݍ���.
	*
	* @param filename �t�H���g�t�@�C����.
	*
	* @retval true  �ǂݍ��ݐ���.
	* @retval false �ǂݍ��ݎ��s.
	*/
	bool Renderer::LoadFromFile(const char* filename){

		// std::unique_ptr�Ƀt�@�C���|�C���^���i�[
		// fclose�֐��Ńt�@�C���|�C���^�̉��
		const std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(filename, "r"), fclose);
		if (!fp) {
			std::cerr << "ERROR: '" << filename << "'�̃I�[�v���Ɏ��s" << std::endl;
			return false;
		}

		// fscanf�֐���1�s���ǂݍ���
		int line = 1; ///< �t�H���g�t�@�C���̏������̍s.
		int ret = fscanf(fp.get(),
			"info face=%*s size=%*d bold=%*d italic=%*d charset=%*s unicode=%*d stretchH=%*d smooth=%*d aa=%*d padding=%*d,%*d,%*d,%*d spacing=%*d,%*d");
		++line;

		// ���O�̉��s�R�[�h���X�L�b�v���邽�߂ɏ���������̐擪��1�����̋󔒂�t�������Ă���.
		// ���̋󔒂��Ȃ��Ɛ������ǂݍ��ނ��Ƃ��ł��Ȃ�.
		glm::vec2 scale;
		ret = fscanf(fp.get(),
			" common lineHeight=%*d base=%*d scaleW=%f scaleH=%f pages=%*d packed=%*d",
			&scale.x, &scale.y);
		if (ret < 2) {
			std::cerr << "ERROR: '" << filename << "'�̓ǂݍ��݂Ɏ��s(line=" << line <<
				")" << std::endl;
			return false;
		}
		const glm::vec2 reciprocalScale(1.0f / scale);
		++line;

		char tex[128];
		ret = fscanf(fp.get(), " page id=%*d file=%127s", tex);
		if (ret < 1) {
			std::cerr << "ERROR: '" << filename << "'�̓ǂݍ��݂Ɏ��s(line=" << line <<
				")" << std::endl;
			return false;
		}
		texFilename.assign(tex + 1, tex + strlen(tex) - 1);
		++line;

		int charCount;
		ret = fscanf(fp.get(), " chars count=%d", &charCount);
		if (ret < 1) {
			std::cerr << "ERROR: '" << filename << "'�̓ǂݍ��݂Ɏ��s(line=" << line <<
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
				std::cerr << "ERROR: '" << filename << "'�̓ǂݍ��݂Ɏ��s(line=" << line <<
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

		// �e�N�X�`����ǂݍ���.
		if (!GameEngine::Instance().LoadTextureFromFile(texFilename.c_str())) {
			return false;
		}
		return true;
	}

	/**
	* @desc		�����F��ݒ肷��.
	*
	* @param	c�����F.
	* @tips		�����œn���ꂽ�F��glm::clamp�֐���0�`1�͈̔͂Ɋۂ߁A
	*			255�{���邱�Ƃ�0�`255�͈̔͂𐶐�.
	*/
	void Renderer::Color(const glm::vec4& c){
		color = glm::clamp(c, 0.0f, 1.0f) * 255.0f;
	}

	/**
	* �������ǉ�����.
	*
	* @param position �\���J�n���W.
	* @param str      �ǉ����镶����.
	*
	* @retval true  �ǉ�����.
	* @retval false �ǉ����s.
	* @tips			�������ƂɈʒu�����擾���AVBO��4�̒��_�f�[�^��ǉ�����
	*/
	bool Renderer::AddString(const glm::vec2& position, const char* str){
		Vertex* p = pVBO + vboSize;
		glm::vec2 pos = position;
		for (const char* itr = str; *itr; ++itr) {
			// VBO�ɒ��_�f�[�^��ǉ����邽�߂̋󂫂��Ȃ��ꍇ�́A���̎��_�Œǉ���ł��؂�.
			if (vboSize + 4 > vboCapacity) {
				break;
			}
			const FontInfo& font = fontList[*itr];

			// id��0�����̏ꍇ�A�ʒu��񂪂Ȃ����Ƃ������Ă��邽�ߕ`�悵�Ȃ�.
			// �܂��A������X��Y�����ꂩ�̃T�C�Y��0�̏ꍇ���\���ł��Ȃ����ߕ`�悵�Ȃ�.
			if (font.id >= 0 && font.size.x && font.size.y) {
				// �f�[�^��VBO�ɒǉ�
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
	* @desc	VBO���V�X�e���������Ƀ}�b�s���O����.
	* @tips	AddString�֐��Œ��_�f�[�^�̊i�[��ƂȂ�VBO����������.
	*/
	void Renderer::MapBuffer(){
		// VBO�ւ̃|�C���^���擾�ς݂��ǂ���
		if (pVBO) {
			return;
		}
		glBindBuffer(GL_ARRAY_BUFFER, vbo.Id());
		// glMapBufferRange�֐��Ńo�b�t�@�I�u�W�F�N�g�ւ̃|�C���^���擾���Ċi�[.
		// ���蓖�Ĕ͈͂��o�b�t�@�S�̂Ƃ��A�t���O��GL_MAP_INVALIDATE_BUFFER_BIT��n�����ƂŐV�����o�b�t�@���쐬���A
		// �Â��o�b�t�@���I���t�@��������.
		pVBO = static_cast<Vertex*>(glMapBufferRange(GL_ARRAY_BUFFER, 0,
			sizeof(Vertex) * vboCapacity, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// �o�b�t�@�̎g�p�ʂ����Z�b�g.
		vboSize = 0;
	}

	/**
	* @desc	VBO�̃}�b�s���O����������.
	* @tips	MapBuffer�Ŏ擾�����o�b�t�@�I�u�W�F�N�g�ւ̃|�C���^��OepnGL�V�X�e���ɕԋp���邱�ƂŁA
	*		���_�f�[�^��GPU�������֓]������悤�Ɏw������.
	*/
	void Renderer::UnmapBuffer(){
		// VBO���쐬���Ă��Ȃ��A���邢��1�������ǉ�����Ă��Ȃ��ꍇ�͉��������ɏI��.
		if (!pVBO || vboSize == 0) {
			return;
		}
		glBindBuffer(GL_ARRAY_BUFFER, vbo.Id());
		// VBO�̊��蓖�Ă�����
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		pVBO = nullptr;
	}

	/**
	* �t�H���g��`�悷��.
	*/
	void Renderer::Draw() const{
		// ������1������������Ă��Ȃ���΁A�`��̕K�v�͂Ȃ��̂ŉ��������ɏI��.
		if (vboSize == 0) {
			return;
		}
		// OpenGL�V�X�e���Ƀt�H���g�p��VAO�����蓖��.
		vao.Bind();
		// ���ׂĂ̕����͓���Z���W�������߁A�㏑���ł���悤�ɐ[�x�e�X�g�𖳌���.
		glDisable(GL_DEPTH_TEST);
		// �����ɂ͓����ȕ���������̂ŁA�A���t�@�u�����h��L��.
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// �t�H���g�p�V�F�[�_�[�v���O���������蓖��.
		progFont->UseProgram();
		// �e�N�X�`�����o�C���h.
		progFont->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D,
			GameEngine::Instance().GetTexture(texFilename.c_str())->Id());
		// �S�Ă̕�����`��.
		glDrawElements(GL_TRIANGLES, (vboSize / 4) * 6, GL_UNSIGNED_SHORT, 0);
		vao.Unbind();
	}
}