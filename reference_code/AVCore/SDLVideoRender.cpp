/**
 * @file SDLVideoRender.cpp
 * @author yilongdong (dyl20001223@163.com)
 * @brief
 * @details
 * @version 0.1
 * @date 2022-05-13
 *
 * @copyright Copyright (c) 2022
 * TODO:
 *  1. 创建一个窗口
 *  2. 创建渲染器
 *  3. 创建纹理
 *  4. 读取帧数据到纹理
 *  5. 刷新渲染器显示内容
 *  https://www.libsdl.org
 *  https://www.cnblogs.com/xl2432/p/11791295.html
 */

extern "C" {
#include <libavformat/avformat.h>

#include "SDL.h"
#include "SDL_main.h"
}
#include "AVCore.h"

SDLVideoRender::SDLVideoRender()
    : pWindow_(nullptr),
      pRender_(nullptr),
      pTexture_(nullptr),
	  pDrawRect_(nullptr),
	  pConverter_(nullptr),
	  WindowWidth(0),
	  WindowHeight(0),
	  VideoRenderWidth(0),
	  VideoRenderHeight(0){
  LOGTRACE("SDLVideoRender: Start");
  SDL_Init(SDL_INIT_VIDEO);
}

SDLVideoRender::~SDLVideoRender() {
  if (pRender_) {
    ClearScreen();
    SDL_DestroyRenderer(pRender_);
  }
  if (pTexture_) {
    SDL_DestroyTexture(pTexture_);
  }
  if (pWindow_) {
    SDL_DestroyWindow(pWindow_);
  }
  SDL_Quit();
}

void SDLVideoRender::Clear(){}

void SDLVideoRender::Init(std::string const& title, uint32_t width,
                          uint32_t height, void* windowHandle) {
  if (pWindow_ == nullptr) {
    if (windowHandle == nullptr) {
      pWindow_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, width, height,
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE);
    } else {
      pWindow_ = SDL_CreateWindowFrom(windowHandle);
    }
  }

  SDL_GetWindowSize(pWindow_, &WindowWidth, &WindowHeight);
  SDL_UpdateWindowSurface(pWindow_);

  pRender_ = SDL_CreateRenderer(pWindow_, -1, 0);
  LOGTRACE_IF(pRender_!=nullptr,"SDLVideoRender: Could not Create Render");

  LOGTRACE("SDLVideoRenderer: SDLWindow pointer Set");
}

void SDLVideoRender::ClearScreen(){
 LOGTRACE("SDLVideoRenderer: ClearScreen");
 SDL_RenderClear(pRender_);
}

void SDLVideoRender::ResizeWindow(){
 LOGTRACE("SDLVideoRenderer: Get New Window Size");
 SDL_GetWindowSize(pWindow_,&WindowWidth,&WindowHeight);
}

void SDLVideoRender::ResetWindow(int _x,int _y,int _w,int _h)
{
  WindowWidth=_w;
  WindowHeight=_h;
  SDL_SetWindowPosition(pWindow_,_x,_y);
  SDL_SetWindowSize(pWindow_,_w,_h);
  SDL_UpdateWindowSurface(pWindow_);
  SDL_RenderSetLogicalSize(pRender_, _w, _h);

  RenderOnWindowFromTexture();
}

void SDLVideoRender::ResizeVideo_FullScreen(){
 VideoRenderWidth=VideoRenderHeight=0;
 LOGTRACE("SDLVideoRenderer: Resize to Default(FullScreen)");
}

void SDLVideoRender::ResizeVideo(int _w,int _h){
 VideoRenderWidth=_w;
 VideoRenderHeight=_h;
 LOGTRACE("SDLVideoRenderer: Resize to (%d,%d)",VideoRenderWidth,VideoRenderHeight);
}

void SDLVideoRender::Render(std::shared_ptr<AV::Frame> pFrame) {
  LOGTRACE("SDLVideoRenderer: Rendering A frame");
  if (pFrame->GetFormat() == -1) {
    return;
  }
  int w = pFrame->frame->width;
  int h = pFrame->frame->height;

  // 因为视频帧转换暂时还有一点bug所以先注释掉。
  // 加这个转换主要是为了增加支持的视频编码种类，不加转换只支持YUV420P格式
  // 把视频帧转换为AV_PIX_FMT_YUV420P格式
  LOGTRACE("from %d w=%d,h=%d to %d w=%d,h=%d", pFrame->frame->format, w, h,
           AV_PIX_FMT_YUV420P, w, h);

  //AV::Converter converter(pFrame->frame->format, AV_PIX_FMT_YUV420P, w, h);
  if(pConverter_ == nullptr) pConverter_ = new AV::Converter(pFrame->frame->format, AV_PIX_FMT_YUV420P, w, h);
  AVFrame* pFrameYUV=av_frame_alloc();
  if (pFrameYUV == nullptr) {
    LOGIMPT("pFrame YUV is nullptr");
    return;
  }
  pFrameYUV->format=AV_PIX_FMT_YUV420P;
  pFrameYUV->width = w;
  pFrameYUV->height = h;

  {
    int ret = av_frame_get_buffer(pFrameYUV,32);
	if(ret<0)
	{
		LOGIMPT("Create YUVFrame Failed");
		av_frame_free(&pFrameYUV);
		return;
	}
  }

  pConverter_->Convert(pFrame,pFrameYUV);
//  std::shared_ptr<AV::Frame> pFrameYUV = pConverter_->Convert(pFrame);

//  std::shared_ptr<AV::Frame> pFrameYUV = pFrame;
  if (pTexture_ == nullptr) {
    pTexture_ = SDL_CreateTexture(pRender_, SDL_PIXELFORMAT_YV12,
                                  SDL_TEXTUREACCESS_STREAMING, w, h);
  }

  SDL_UpdateYUVTexture(pTexture_, nullptr, pFrameYUV->data[0],
                       pFrameYUV->linesize[0], pFrameYUV->data[1],
                       pFrameYUV->linesize[1], pFrameYUV->data[2],
                       pFrameYUV->linesize[2]);
  RenderOnWindowFromTexture();

  av_frame_free(&pFrameYUV);
  av_freep(&pFrameYUV);
}

inline void SDLVideoRender::RenderOnWindowFromTexture()
{
  if(pDrawRect_ == nullptr) pDrawRect_=new SDL_Rect;
  if(!VideoRenderWidth||VideoRenderWidth>WindowWidth)
  {
   pDrawRect_->x = 0; pDrawRect_->y = 0;
   pDrawRect_->w = WindowWidth;
   pDrawRect_->h = WindowHeight;
  }
  else
  {
   pDrawRect_->x = (WindowWidth-VideoRenderWidth)>>1;
   pDrawRect_->y = (WindowHeight-VideoRenderHeight)>>1;
   pDrawRect_->w = VideoRenderWidth;
   pDrawRect_->h = VideoRenderHeight;
  }

  SDL_RenderClear(pRender_);
  SDL_RenderCopy(pRender_, pTexture_, nullptr, pDrawRect_);
  SDL_RenderPresent(pRender_);
}
