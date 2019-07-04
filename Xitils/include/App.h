#pragma once

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include <thread>

#define XITILS_APP(x) CINDER_APP(x, RendererGl)

namespace Xitils::App {

	template<typename T> class XApp : public ci::app::App {
	public:
		virtual void OnSetup(T* frameParams) {}
		virtual void OnCleanUp() {}
		virtual void OnUpdate(T* frameParams) {} // ���C���X���b�h�ȊO����Ă΂��̂Œ���
		virtual void OnDraw(const T& frameParams) {} // ������̓��C���X���b�h����Ă΂��

		void setup() override;
		void draw() override;
		void cleanup() override;
		void update() override;

		void mainLoop();

	protected:
	private:
		// �X�V�p�X���b�h�ƕ`��p�X���b�h����������肷��ہA
		// frameParams ���̃����o�ϐ��Ƃ��ăf�[�^��n���K�v������B
		// frameParams �̓o�b�t�@����ăX���b�h�Z�[�t�ɏ��������B
		// ����ȊO�̏��� OnUpdate �� OnDraw �ł܂����Ŏg���Ă͂����Ȃ��B
		T frameParams;

		T frameParamsBuffer;
		std::mutex mtx;

		ci::Surface surfaceBuffer;
		ci::gl::TextureRef texture;
		float elapsedBuffer = 0.0f;

		std::shared_ptr<std::thread> mainLoopThread;
		bool threadClosing = false;

	};

	template<typename T> void XApp<T>::setup() {
		OnSetup(&frameParams);

		mainLoopThread = std::make_shared<std::thread>([&] {
			this->mainLoop();
			});
	}

	template<typename T> void XApp<T>::draw() {
		std::lock_guard lock(mtx);
		OnDraw(frameParamsBuffer);
	}

	template<typename T> void XApp<T>::cleanup() {
		if (mainLoopThread) {
			threadClosing = true;
			mainLoopThread->join();
		}
	}

	template<typename T> void XApp<T>::update() {
	}

	template<typename T> void XApp<T>::mainLoop() {
		float elapsed = 0.0f;

		while (true) {
			OnUpdate(&frameParams);

			{
				std::lock_guard lock(mtx);
				frameParamsBuffer = frameParams;
			}

			std::this_thread::yield();

			if (threadClosing) { break; }
		}
	}

}