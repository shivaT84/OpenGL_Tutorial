/**
* @file OffscreenBuffer.cpp
*/
#include "OffscreenBuffer.h"

/**
* �I�t�X�N���[���o�b�t�@���쐬����.
*
* @param w �I�t�X�N���[���o�b�t�@�̕�(�s�N�Z���P��).
* @param h �I�t�X�N���[���o�b�t�@�̍���(�s�N�Z���P��).
*
* @return �쐬�����I�t�X�N���[���o�b�t�@�ւ̃|�C���^.
*/
OffscreenBufferPtr OffscreenBuffer::Create(int w, int h){
	struct Impl : OffscreenBuffer {
		Impl() {};	// �R���X�g���N�^
		~Impl() {};	// �f�X�g���N�^
	};
	OffscreenBufferPtr offscreen = std::make_shared<Impl>();

	if (!offscreen) {
		return offscreen;
	}

	// �t���[���o�b�t�@�p�e�N�X�`���̍쐬
	offscreen->tex = Texture::Create(w, h, GL_RGBA8, GL_RGBA, nullptr);

	// �[�x�o�b�t�@�̍쐬
	glGenRenderbuffers(1, &offscreen->depthbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, offscreen->depthbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// �t���[���o�b�t�@�Ƃ��ē���
	glGenFramebuffers(1, &offscreen->framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, offscreen->framebuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, offscreen->depthbuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offscreen->tex->Id(), 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return offscreen;
}


/**
* �f�X�g���N�^.
*/
OffscreenBuffer::~OffscreenBuffer(){
	if (framebuffer) {
		glDeleteFramebuffers(1, &framebuffer);
	}
	if (depthbuffer) {
		glDeleteRenderbuffers(1, &depthbuffer);
	}
}