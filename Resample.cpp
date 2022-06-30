#include <iostream>

#include "AVCore.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

Resample::Resample()
{
	pswrContext = swr_alloc();
	if (!pswrContext) {
		std::cout << "swr_alloc context fail" << std::endl;
	}
	std::cout << "construct resample" << std::endl;

}

Resample::~Resample()
{
	if (pswrContext) {
		swr_free(&pswrContext);
		pswrContext = nullptr;
	}
	std::cout << "deconstruct resample" << std::endl;
}

int Resample::close()
{
	std::unique_lock<std::mutex> locker(mtx);
	std::cout << "resample close" << std::endl;
	if (pswrContext) 
	{
		swr_free(&pswrContext);
		pswrContext = nullptr;
	}
	return 0;
}

int Resample::open(AVCodecParameters* para)
{
	std::unique_lock<std::mutex> locker(mtx);
	codecPara = para;
	std::cout << "Resample Open" << std::endl;
	if (!para) {
		std::cout << "AVCodecParameters is nullptr" << std::endl;
	}
	std::cout << "Set Context Options" << std::endl;
	pswrContext = swr_alloc_set_opts(pswrContext, av_get_default_channel_layout(para->channels),
		(AVSampleFormat)outputFormat,
		para->sample_rate,
		av_get_default_channel_layout(para->channels),
		(AVSampleFormat)para->format,
		para->sample_rate, 0, 0);
	channel_num = para->channels;
	std::cout << "Inti Context" << std::endl;
	int ret = swr_init(pswrContext);
	if (ret) {
		std::cout << "Init resample context error!" << std::endl;
		return ret;
	}
}

int Resample::getOriginalSampleRate()
{
	std::unique_lock<std::mutex> locker(mtx);
	return codecPara->sample_rate;
}

int Resample::doResample(std::shared_ptr<Frame> pFrame,
		std::vector<uint8_t>& pcm) {
	std::unique_lock<std::mutex> locker(mtx);
	pcm.resize(1024 * 1024 * 10);
	if (!pFrame || !pFrame->frame) {
		std::cout << "Frame ptr is nullptr" << std::endl;
	}
	uint8_t* data[2] = { 0 };//std::vector::data() return a pointer
	data[0] = pcm.data();
	data[1] = pcm.data();
	std::cout << "Swr convert begin" << std::endl;
	if (pswrContext == nullptr) {
		std::cout << "swrContext is nullptr" << std::endl;
	}
	int number_of_sample_output_per_channel =
		swr_convert(pswrContext,
			data, pFrame->frame->nb_samples,
			(uint8_t const**)(pFrame->frame->data),
			pFrame->frame->nb_samples);
	if (number_of_sample_output_per_channel < 0) {
		std::cout << "do resample error! ret = " <<
			number_of_sample_output_per_channel << std::endl;
	}
	std::cout << "Swr convert end" << std::endl;
	pcm.resize(number_of_sample_output_per_channel * pFrame->frame->channels *
		av_get_bytes_per_sample((AVSampleFormat)outputFormat));

	return 0;
}

int Resample::setOutputFormat(uint32_t format)
{
	std::unique_lock<std::mutex> locker(mtx);
	outputFormat = format;
	std::cout << "Set output format = " << format <<std::endl;
	return 0;
}

int Resample::setDefaultOutputFormat()
{
	std::unique_lock<std::mutex> locker(mtx);
	outputFormat = AV_SAMPLE_FMT_S16;
	std::cout << "Set default output format = " << outputFormat << std::endl;
	return 0;
}

int Resample::clear() { return 0; }

int Resample::setSpeedRate(double speed_rate)
{
	std::unique_lock<std::mutex> locker(mtx);
	std::cout << "set speed rate = " << speed_rate << std::endl;
	static double epsilon = 0.01;
	//do not understand
	if (speed_rate < epsilon || std::abs(speed_rate - 1.0) < epsilon ||
		speed_rate < 0) {
		sample_rate = codecPara->sample_rate;
	}
	else {
		sample_rate = 1.0 * codecPara->sample_rate / speed_rate;
	}

	std::cout << "original sample rate = " << codecPara->sample_rate
		<< "current sample rate = " << sample_rate << std::endl;

	if (pswrContext) {
		swr_free(&pswrContext);
		pswrContext = nullptr;
	}

	pswrContext = swr_alloc();
	if (pswrContext == nullptr) {
		std::cout << "swr_alloc context failed" << std::endl;
	}
	if (!codecPara) {
		std::cout << "codecPara is nullptr" << std::endl;
	}

	std::cout << "Set Context Options" << std::endl;
	pswrContext = swr_alloc_set_opts(
		pswrContext, av_get_default_channel_layout(2),
		(AVSampleFormat)outputFormat,
		sample_rate,
		av_get_default_channel_layout(codecPara->channels),
		(AVSampleFormat)codecPara->format,
		codecPara->sample_rate, 0, 0
	);
	std::cout << "Init Context" << std::endl;
	int ret = swr_init(pswrContext);
	if (ret) {
		std::cout << "Init resample context error! ret = " << ret << std::endl;
	}
	return 0;
}