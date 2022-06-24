# AVFrame 结构分析
AVFrame结构体一般用于存储原始数据（即非压缩数据，例如对视频来说是YUV，RGB，对音频来说是PCM）

```cpp
uint8_t *data[AV_NUM_DATA_POINTERS]：解码后原始数据（对视频来说是YUV，RGB，对音频来说是PCM）

int linesize[AV_NUM_DATA_POINTERS]：data中“一行”数据的大小。注意：未必等于图像的宽，一般大于图像的宽。

int width, height：视频帧宽和高（1920 1080）

int nb_samples：音频的一个AVFrame中可能包含多个音频帧，在此标记包含了几个

int format：解码后原始数据类型（YUV420，YUV422，RGB24...）

int key_frame：是否是关键帧

enum AVPictureType pict_type：帧类型（I,B,P...）

AVRational sample_aspect_ratio：宽高比（16:9）

int64_t pts：显示时间戳

int coded_picture_number：编码帧序号

int display_picture_number：显示帧序号

int8_t *qscale_table：QP表

uint8_t *mbskip_table：跳过宏块表

int16_t (*motion_val[2])[2]：运动矢量表

uint32_t *mb_type：宏块类型表

short *dct_coeff：DCT系数

int8_t *ref_index[2]：运动估计参考帧列表

int interlaced_frame：是否是隔行扫描

uint8_t motion_subsample_log2：一个宏块中的运动矢量采样个数，取log的
```