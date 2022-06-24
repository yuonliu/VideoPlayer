/**
 * @file TestSDL.cpp
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
 *  https://www.libsdl.org/download-2.0.php
 */

extern "C" {
#include "SDL.h"
#include "SDL_main.h"
}
#include <thread>
// #define SDL_MAIN_HANDLED

// main函数需要写全，不然会报"SDL2main.lib(SDL_windows_main.obj) : error
// LNK2019: 无法解析的外部符号 SDL_main，函数 main_getcmdline
// 中引用了该符号"的错误
int main(int argc, char* argv[]) {
  SDL_Init(SDL_INIT_VIDEO);
  // 创建窗口
  // https://blog.csdn.net/leixiaohua1020/article/details/40701203
  SDL_Window* window = SDL_CreateWindow(
      "SDL2 Test Window", 100, 100, 640, 480,
      SDL_WINDOW_SHOWN |
          SDL_WINDOW_BORDERLESS);  // 包括了窗口的是否最大化、最小化，能否调整边界等等属性
  // 创建渲染器
  // https://blog.csdn.net/leixiaohua1020/article/details/40723085
  SDL_Renderer* render = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  // 渲染一个黄色的背景
  SDL_SetRenderDrawColor(render, 255, 255, 0, 255);

  // 清理屏幕
  SDL_RenderClear(render);

  // 进行绘制
  SDL_RenderPresent(render);

  // 暂停一下

  // SDL_Delay(5000);
  std::this_thread::sleep_for(std::chrono::seconds(5));

  //运行结束 销毁
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}