/**
 * @file Log.h
 * @author yilongdong (dyl20001223@163.com)
 * @date 2022-04-27
 * @brief 简单的日志宏和计时工具, 使用printf, 线程安全
 * @version 0.1
 * @copyright Copyright (c) 2022
 * @details 介绍
 * #define DEBUG               开启ONTEST日志
 * #define LOG_EN [num]        设置日志输出等级
 * #define LOGERROR_CODE [num] 设置ERROR日志退出码，默认-1
 * ERROR日志会在输出后调用std::exit(LOGERROR_CODE)
 * TODO: 1. 支持日志输出到文件 2. 提到公共工具的文件夹下
 */

#pragma once
#include <cstdio>
#include <cstdlib>
// #define DEBUG
#define LOG_EN 2

#ifndef LOG_EN
#define LOG_EN 1  //默认只打印ERROR日志
#endif

#define LOGTIPS "[%s:%d->%s] "
#if (LOG_EN >= 1)
#define LOG_EN_ERROR
#endif
#if (LOG_EN >= 2)
#define LOG_EN_IMPT
#endif
#if (LOG_EN >= 3)
#define LOG_EN_WARN
#endif

#if (LOG_EN >= 3)
#define LOG_EN_INFO
#endif
#if (LOG_EN >= 5)
#define LOG_EN_TRACE
#endif

#define RESETCOLOR "\033[0m"
#define REDCOLOR "\033[1;31m"
#define YELLOWCOLOR "\033[1;33m"
#define GREENCOLOR "\033[1;32m"
#define CYANCOLOR "\033[1;36m"
#define STR(x) #x

#define LOGERROR_CODE (-1)

#ifndef LOG_OUTPUT
#define LOG_OUTPUT stderr
#endif

#ifdef LOG_EN_ERROR
#define ERRORLOGTIPS RESETCOLOR REDCOLOR "ERROR: [%s:%d->%s] " RESETCOLOR
#define LOGERROR(format, ...)                                         \
  {                                                                   \
    fprintf(LOG_OUTPUT, ERRORLOGTIPS format "\n", __FILE__, __LINE__, \
            __func__, ##__VA_ARGS__);                                 \
    std::exit(LOGERROR_CODE);                                         \
  }
#define LOGERROR_IF(expr, format, ...)                                         \
  if (expr) {                                                                  \
    fprintf(LOG_OUTPUT, ERRORLOGTIPS format "\t with " STR(expr) " is true\n", \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__);                      \
    std::exit(LOGERROR_CODE);                                                  \
  }
#else
#define ERRORLOGTIPS ""
#define LOGERROR(format, ...) ((void)0)
#define LOGERROR_IF(expr, format, ...) ((void)0)
#endif

#ifdef LOG_EN_WARN
#define WARNLOGTIPS RESETCOLOR YELLOWCOLOR "WARN: [%s:%d->%s] " RESETCOLOR
#define LOGWARN(format, ...)                                                 \
  fprintf(LOG_OUTPUT, WARNLOGTIPS format "\n", __FILE__, __LINE__, __func__, \
          ##__VA_ARGS__);
#define LOGWARN_IF(expr, format, ...)                                          \
  if (expr) {                                                                  \
    fprintf(LOG_OUTPUT, ERRORLOGTIPS format "\t with " STR(expr) " is true\n", \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__);                      \
  }
#else
#define WARNLOGTIPS ""
#define LOGWARN(format, ...) ((void)0)
#define LOGWARN_IF(expr, format, ...) ((void)0)
#endif

#ifdef LOG_EN_IMPT
#define IMPTLOGTIPS RESETCOLOR REDCOLOR "IMPT: [%s:%d->%s] " RESETCOLOR
#define LOGIMPT(format, ...)                                                 \
  fprintf(LOG_OUTPUT, IMPTLOGTIPS format "\n", __FILE__, __LINE__, __func__, \
          ##__VA_ARGS__);
#define LOGIMPT_IF(expr, format, ...)                                          \
  if (expr) {                                                                  \
    fprintf(LOG_OUTPUT, ERRORLOGTIPS format "\t with " STR(expr) " is true\n", \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__);                      \
  }
#else
#define IMPTLOGTIPS ""
#define LOGIMPT(format, ...) ((void)0)
#define LOGIMPT_IF(expr, format, ...) ((void)0)
#endif

#ifdef LOG_EN_INFO
#define INFOLOGTIPS RESETCOLOR GREENCOLOR "INFO: [%s:%d->%s] " RESETCOLOR
#define LOGINFO(format, ...)                                                 \
  fprintf(LOG_OUTPUT, INFOLOGTIPS format "\n", __FILE__, __LINE__, __func__, \
          ##__VA_ARGS__);
#define LOGINFO_IF(expr, format, ...)                                          \
  if (expr) {                                                                  \
    fprintf(LOG_OUTPUT, ERRORLOGTIPS format "\t with " STR(expr) " is true\n", \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__);                      \
  }
#else
#define INFOLOGTIPS ""
#define LOGINFO(format, ...) ((void)0)
#define LOGINFO_IF(expr, format, ...) ((void)0)
#endif

#ifdef LOG_EN_TRACE
#define TRACELOGTIPS RESETCOLOR "TRACE: [%s:%d->%s] " RESETCOLOR
#define LOGTRACE(format, ...)                                                 \
  fprintf(LOG_OUTPUT, TRACELOGTIPS format "\n", __FILE__, __LINE__, __func__, \
          ##__VA_ARGS__);
#define LOGTRACE_IF(expr, format, ...)                                         \
  if (expr) {                                                                  \
    fprintf(LOG_OUTPUT, ERRORLOGTIPS format "\t with " STR(expr) " is true\n", \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__);                      \
  }
#else
#define TRACELOGTIPS ""
#define LOGTRACE(format, ...) ((void)0)
#define LOGTRACE_IF(expr, format, ...) ((void)0)
#endif

#ifdef DEBUG
#define ONTESTLOGTIPS RESETCOLOR CYANCOLOR "ONTEST: [%s:%d->%s] " RESETCOLOR
#define LOGONTEST(format, ...)                                                 \
  fprintf(LOG_OUTPUT, ONTESTLOGTIPS format "\n", __FILE__, __LINE__, __func__, \
          ##__VA_ARGS__);
#define LOGONTEST_IF(expr, format, ...)                                        \
  if (expr) {                                                                  \
    fprintf(LOG_OUTPUT, ERRORLOGTIPS format "\t with " STR(expr) " is true\n", \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__);                      \
  }
#else
#define ONTESTLOGTIPS ""
#define LOGONTEST(format, ...) ((void)0)
#define LOGONTEST_IF(expr, format, ...) ((void)0)
#endif

#include <chrono>
#include <string>
class ClockGuardImpl {
 public:
  ClockGuardImpl(std::string const& filename, std::string const& func,
                 int lineno, std::string const& msg = "")
      : filename_(filename), func_(func), lineno_(lineno), msg_(msg) {
    t1_ = std::chrono::system_clock::now();
  }
  ~ClockGuardImpl() {
    t2_ = std::chrono::system_clock::now();
    std::chrono::milliseconds elapsed_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(t2_ - t1_);
    printf(RESETCOLOR CYANCOLOR "CLOCKER: [%s:%d->%s] " RESETCOLOR
                                "Elapsed time = %lld ms, with msg = %s",
           filename_.c_str(), lineno_, func_.c_str(), elapsed_time.count(),
           msg_.c_str());
  }

 private:
  std::chrono::time_point<std::chrono::system_clock> t1_;
  std::chrono::time_point<std::chrono::system_clock> t2_;
  std::string msg_;
  std::string filename_;
  std::string func_;
  int lineno_;
};

#define LOG_COLOCKER(msg) \
  ClockGuardImpl logClocker(__FILE__, __func__, __LINE__, msg);
