extern "C" {
#include <libavformat/avformat.h>
#include <SDL.h>
#include <SDL_main.h>

}

#include "AVCore.h"

SDLVideoRender::SDLVideoRender()
	:pWindow(nullptr),
	pRender(nullptr),
	pTexture(nullptr),
	pDrawRect(nullptr),
	pConverter(nullptr),
	windowWidth(0),
	windowHeight(0),
	videoRenderWidth(0),
	videoRenderHeight(0) {
	std::cout << "SDLVideoRender: Start" << std::endl;
	SDL_Init(SDL_INIT_VIDEO);
}

SDLVideoRender::~SDLVideoRender()
{
	if (pRender) {
		clearScreen();
		SDL_DestroyRenderer(pRender);
	}
	if (pTexture) {
		SDL_DestroyTexture(pTexture);
	}
	if (pWindow) {
		SDL_DestroyWindow(pWindow);
	}
	SDL_Quit();
}

void SDLVideoRender::clear(){}

void SDLVideoRender::init(std::string const& title, uint32_t weight, uint32_t height, void* windowHandle)
{
	if (pWindow == nullptr) {
		if (windowHandle == nullptr) {
			pWindow = SDL_CreateWindow(title.c_str(),
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				weight,
				height,
				SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE
			);
		}
		else {
			pWindow = SDL_CreateWindowFrom(windowHandle);
		}
	}
	SDL_GetWindowSize(pWindow, &windowWidth, &windowHeight);
	SDL_UpdateWindowSurface(pWindow);

	pRender = SDL_CreateRenderer(pWindow, -1, 0);
	if (pRender != nullptr) {
		std::cout << "SDLVideoRender: Could not Create Render" << std::endl;
	}
	std::cout << "SDLVideoRender:: SDLWindow pointer set" << std::endl;
}

void SDLVideoRender::clearScreen()
{
	std::cout << "SDLVideoRender::clearScreen" << std::endl;
	SDL_RenderClear(pRender);
}

void SDLVideoRender::resizeWindow()
{
	std::cout << "SDLVideoRender: Get New Window Size" << std::endl;
	SDL_GetWindowSize(pWindow, &windowWidth, &windowHeight);
}

void SDLVideoRender::resetWindow(int x, int y, int w, int h)
{
	windowWidth = w;
	windowHeight = h;
	SDL_SetWindowPosition(pWindow, x, y);
	SDL_SetWindowSize(pWindow, w, h);
	SDL_UpdateWindowSurface(pWindow);
	SDL_RenderSetLogicalSize(pRender, w, h);

	renderOnWindowFromTexture();
}

void SDLVideoRender::resizeVideo_FullScreeen()
{
	videoRenderWidth = videoRenderHeight = 0;
	std::cout << "SDLVideoRender: Resize to Default" << std::endl;
}

void SDLVideoRender::resizeVideo(int w, int h)
{
	videoRenderWidth = w;
	videoRenderHeight = h;
	std::cout << "SDLVideoRender: Resize to " << videoRenderWidth <<
		" " << videoRenderHeight << std::endl;
}

void SDLVideoRender::render(std::shared_ptr<Frame> pFrame)
{
	std::cout << "SDLVideoRender: Rendering a Frame" << std::endl;

	if (pFrame->getFormat() == -1) {
		return;
	}
	int w = pFrame->frame->width;
	int h = pFrame->frame->height;

	if (pConverter == nullptr) {
		pConverter = new Converter(pFrame->frame->format, AV_PIX_FMT_YUV420P, w, h);
	}

	AVFrame* pFrameYUV = av_frame_alloc();
	if (pFrameYUV == nullptr) {
		std::cout << "pFrame YUV is nullptr" << std::endl;
		return;
	}

	pFrameYUV->format = AV_PIX_FMT_YUV420P;
	pFrameYUV->width = w;
	pFrameYUV->height = h;

	{
		int ret = av_frame_get_buffer(pFrameYUV, 32);
		if (ret < 0)
		{
			std::cout << "Create YUVFrame Failed" << std::endl;
			av_frame_free(&pFrameYUV);
			return;
		}
	}

	pConverter->Convert(pFrame, pFrameYUV);

	if (pTexture == nullptr)
	{
		pTexture = SDL_CreateTexture(pRender, SDL_PIXELFORMAT_YV12,
			SDL_TEXTUREACCESS_STREAMING, w, h);
	}
	//do not understand
	SDL_UpdateYUVTexture(pTexture, nullptr,
		pFrameYUV->data[0], pFrameYUV->linesize[0],
		pFrameYUV->data[1], pFrameYUV->linesize[1],
		pFrameYUV->data[2], pFrameYUV->linesize[2]);

	renderOnWindowFromTexture();

	av_frame_free(&pFrameYUV);
	av_freep(&pFrameYUV);

}

inline void SDLVideoRender::renderOnWindowFromTexture()
{
	if (pDrawRect == nullptr) pDrawRect = new SDL_Rect;
	if (!videoRenderWidth || videoRenderWidth > windowWidth)
	{
		pDrawRect->x = 0;
		pDrawRect->y = 0;
		pDrawRect->w = windowWidth;
		pDrawRect->x = windowHeight;
	}
	else {
		//do not understand
		pDrawRect->x = (windowWidth - videoRenderWidth) >> 1;
		pDrawRect->y = (windowHeight - videoRenderHeight) >> 1;
		pDrawRect->w = videoRenderWidth;
		pDrawRect->h = videoRenderHeight;
	}

	SDL_RenderClear(pRender);
	SDL_RenderCopy(pRender, pTexture, nullptr, pDrawRect);
	SDL_RenderPresent(pRender);

}

