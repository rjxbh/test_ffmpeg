//#include <iostream>
//
//extern "C"
//{
//#include <libavcodec/avcodec.h>
//#include <libavdevice/avdevice.h>
//#include <libavformat/avformat.h>
//#include<libavformat/avio.h>
//#include <libavutil/avutil.h>
//#include <libavutil/opt.h>
//#include <libavutil/imgutils.h>
//#include <libavutil/frame.h>
//#include <libavutil/time.h>
//#include <libswscale/swscale.h>
//#include <libavfilter/avfilter.h>
//#include <libavfilter/buffersink.h>
//#include <libavfilter/buffersrc.h>
//}
//
//using namespace std;
//
//AVFormatContext* g_inputContext = NULL;
//AVFormatContext* g_outputContext;
//int64_t g_lastReadPacktTime = 0;
//
//
//int  interrupt_cb(void* ctx) {
//	if ((av_gettime() - g_lastReadPacktTime) > 100 * 1000 * 1000) {//100s超时退出
//		std::cout << "主码流断开" << std::endl;
//		return AVERROR_EOF;
//	}
//	return 0;
//}
//
////打开媒体输入文件
//int OpenInputMediaFile(const char* inputUrl) {
//	//创建一个AVFormatContext类型变量并初始化默认参数
//	g_inputContext = avformat_alloc_context();
//
//	// 取得时间（单位为microsecond,微秒）
//	g_lastReadPacktTime = av_gettime();
//
//	// 填写检测是否中止阻塞功能的回调函数。
//	g_inputContext->interrupt_callback.callback = interrupt_cb;
//
//	// 强制以指定格式打开，为NULL时，自动检测其格式。
//	AVInputFormat* fmt = NULL;
//
//	// 强制以指定格式打开，为NULL时，自动检测其格式。
//	AVDictionary** options = NULL;
//	//av_dict_set(options, "rtsp_transport", "tcp", 0);
//	//av_dict_set(options, "stimeout", "3000000", 0);
//	
//	// 打开一个输入流(本地文件名或者是网络流地址URL)，并解析其头部格式，并把信息填写到g_inputContext中。
//	//int ret = avformat_open_input(&g_inputContext, inputUrl.c_str(), fmt, options);
//	int ret = avformat_open_input(&g_inputContext, inputUrl, fmt, options);
//	cout << inputUrl << endl;
//	if (ret != 0)	{
//		cout << "input error" <<ret<<endl;
//		return  ret;
//	}
//	cout << "input successful" << endl;
//	// 读取媒体文件的数据包，分析其中的流信息。分析其帧率
//	ret = avformat_find_stream_info(g_inputContext, nullptr);
//	if (ret < 0) {
//		cout << "get stream information failed"<<endl;
//		return ret;
//	}
//	cout << "get stream information sucessful" << endl;
//	return ret;
//}
//
////打开媒体输出文件
//int OpenOutputMediaFile(const char* outUrl) {
//	// 创建一个用于输出的格式信息对像（AVFormatContext）
//	// 格式指定jpeg(对于OpenCV mat数据，之后处理)
//	int ret = avformat_alloc_output_context2(&g_outputContext, nullptr, "singlejpeg", outUrl);
//	if (ret < 0) {
//		cout << "output error" << endl;
//		goto Error;
//	}
//	cout << "output sucessful" << endl;
//	// 打开文件
//	ret = avio_open2(&g_outputContext->pb, outUrl, AVIO_FLAG_WRITE, nullptr, nullptr);
//	cout<< outUrl<<" "<< g_outputContext->pb << endl;
//	if (ret < 0) {
//		cout << "open error" << endl;
//		goto Error;
//	}
//	cout << "open sucessful" << endl;
//	// 找出源媒体中流的信息
//	for (int i = 0; i < g_inputContext->nb_streams; i++) {
//
//		// 由于是输出图片，所以音频流需要。
//		if (g_inputContext->streams[i]->codec->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO) {
//			continue;
//		}
//		// 用输入流的编码信息来创建一个流对像，
//		AVStream* stream = avformat_new_stream(g_outputContext, g_inputContext->streams[i]->codec->codec);
//
//		// 把输入流的编码信息拷贝到个输出流对的编码信息中，
//		ret = avcodec_copy_context(stream->codec, g_inputContext->streams[i]->codec);
//		if (ret < 0) {
//			cout << "copy failed" << endl;
//			goto Error;
//		}
//		cout << "copy sucessful" << endl;
//	}
//
//	// 分配流的私有信息，并把流的头部信息写到输出文件
//	ret = avformat_write_header(g_outputContext, nullptr);
//	if (ret != 0) {
//		cout << "write header into outfile failed" << endl;
//		goto Error;
//	}
//	cout << "write header into outfile sucessful" << endl;
//	return ret;
//
//Error:
//	// 出错后关闭输出环境变量
//	if (g_outputContext) {
//		for (int i = 0; i < g_outputContext->nb_streams; i++) {
//			avcodec_close(g_outputContext->streams[i]->codec);
//		}
//		avformat_close_input(&g_outputContext);
//	}
//	return ret;
//}
//
////读入一帧数据
//shared_ptr<AVPacket> ReadPacketFrame()  {
//	shared_ptr<AVPacket> packet(static_cast<AVPacket*>(
//		av_malloc(sizeof(AVPacket))), [&](AVPacket* p) {
//			av_packet_free(&p);
//			av_freep(&p);
//		}
//	);
//
//	// 初始化packet
//	av_init_packet(packet.get());
//
//	// 取时间
//	g_lastReadPacktTime = av_gettime();
//
//	// 读入一帧数据到packet
//	int ret = av_read_frame(g_inputContext, packet.get());
//	if (ret == 0) {
//		cout << "read first num sucessful" << endl;
//		return packet;
//	}
//	else {
//		return nullptr;
//	}
//}
//
////把数据写到输出文件中
//void av_packet_rescale_ts(AVPacket* pkt, AVRational src_tb, AVRational dst_tb) {
//	if (pkt->pts != AV_NOPTS_VALUE)
//		pkt->pts = av_rescale_q(pkt->pts, src_tb, dst_tb);
//	if (pkt->dts != AV_NOPTS_VALUE)
//		pkt->dts = av_rescale_q(pkt->dts, src_tb, dst_tb);
//	if (pkt->duration > 0)
//		pkt->duration = av_rescale_q(pkt->duration, src_tb, dst_tb);
//}
//
//int WriteFrame(shared_ptr<AVPacket> packet) {
//	auto inputStream = g_inputContext->streams[packet->stream_index];
//	auto outputStream = g_outputContext->streams[packet->stream_index];
//	av_packet_rescale_ts(packet.get(), inputStream->time_base, outputStream->time_base);
//	return av_interleaved_write_frame(g_outputContext, packet.get());
//}
//
//// 初始化FFMpeg库
//void InitFFMpeg() {
//	av_register_all();    // 注册所有的编解码器
//	avfilter_register_all();// 注册所有的滤镜
//
//	// 初网络组件。如果输入输出文件，不涉及网络，可以不调用。
//	avformat_network_init();
//}
//
////关闭资源
//void CloseInput() {
//	if (g_inputContext != nullptr) {
//		// 关闭输入环境，释放结构中的资源
//		avformat_close_input(&g_inputContext);
//	}
//}
//
//void CloseOutput(){
//	if (g_outputContext != nullptr) {
//		for (int i = 0; i < g_outputContext->nb_streams; i++) {
//			AVCodecContext* codecContext = g_outputContext->streams[i]->codec;
//			// 关闭输出编码器，释放相关的数据。
//			avcodec_close(codecContext);
//		}
//		// 关闭输出环境，释放结构中的资源
//		avformat_close_input(&g_outputContext);
//	}
//}
//
////初始化编码器
//int InitDecodeContext(AVStream* inputStream) {
//	auto codecId = inputStream->codec->codec_id;
//	cout << "codeid："<<codecId << endl;
//	auto codec = avcodec_find_decoder(codecId);
//	cout << "codec：" << codec << endl;
//	if (!codec) {
//		cout << "find codec failed" << endl;
//		return -1;
//	}
//	cout << "find codec sucessful" << endl;
//	int ret = avcodec_open2(inputStream->codec, codec, NULL);
//	return ret;
//}
//
////初始化解码器
//int initEncoderCodec(AVStream* inputStream, AVCodecContext** encodeContext) {
//	AVCodec* picCodec;
//
//	picCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);//获取编码器
//	(*encodeContext) = avcodec_alloc_context3(picCodec);//申请AVCodecContext空间
//	(*encodeContext)->codec_id = picCodec->id;
//	(*encodeContext)->time_base.num = inputStream->codec->time_base.num;
//	(*encodeContext)->time_base.den = inputStream->codec->time_base.den;
//	(*encodeContext)->pix_fmt = *picCodec->pix_fmts;//编码器格式
//	(*encodeContext)->width = inputStream->codec->width;
//	(*encodeContext)->height = inputStream->codec->height;
//	cout << "codec_id:" << (*encodeContext)->codec_id << " " << "num/den:" << (*encodeContext)->time_base.num << "/" << (*encodeContext)->time_base.den << " ";
//	cout << (*encodeContext)->pix_fmt << " " << (*encodeContext)->width << "/"<<(*encodeContext)->height << endl;
//	int ret = avcodec_open2((*encodeContext), picCodec, nullptr);
//	if (ret != 0) {
//		cout << "open video codec failed" << endl;
//		return  ret;
//	}
//	cout << "open video codec sucessful" << endl;
//	return 1;
//}
//
////解码视频帧
//bool Decode(AVStream* inputStream, AVPacket* packet, AVFrame* frame) {
//	int gotFrame = 0;
//	//压缩视频的解码
//	auto hr = avcodec_decode_video2(inputStream->codec, frame, &gotFrame, packet);
//	if (hr >= 0 && gotFrame != 0) {
//		cout << hr << "   " << gotFrame << endl;
//		cout << "decode sucessful" << endl;
//		return true;
//	}
//	cout << "decode failed" << endl;
//	return false;
//}
//
////编码图片数据
//std::shared_ptr<AVPacket> Encode(AVCodecContext* encodeContext, AVFrame* frame) {
//	int gotOutput = 0;
//	std::shared_ptr<AVPacket> pkt(static_cast<AVPacket*>(av_malloc(sizeof(AVPacket))), [&](AVPacket* p) { av_packet_free(&p); av_freep(&p); });
//	av_init_packet(pkt.get());
//	pkt->data = NULL;
//	pkt->size = 0;
//	int ret = avcodec_encode_video2(encodeContext, pkt.get(), frame, &gotOutput);
//	if (ret == 0 && gotOutput) {
//		cout << "encode sucessful" << endl;
//		return pkt;
//	}
//	else{ 
//		cout << "encode failed" << endl;
//		return nullptr;
//	}
//}
//
////主流程
//int CaptureJpeg(const char* mediaURL, const  char* jpegFile) {
//	InitFFMpeg();
//	int ret = OpenInputMediaFile(mediaURL);
//
//	if (ret == 0) 
//		ret = OpenOutputMediaFile(jpegFile);
//	if (ret < 0) {
//		cout << "ERROR" << endl;
//		CloseInput();
//		CloseOutput();
//	}
//
//	AVCodecContext* encodeContext = nullptr;
//	InitDecodeContext(g_inputContext->streams[0]);
//	AVFrame* videoFrame = av_frame_alloc();
//	initEncoderCodec(g_inputContext->streams[0], &encodeContext);
//
//	int num = 0;
//	while (true) {
//		// 从输入流中读取一个数据包
//		auto packet = ReadPacketFrame();
//		// 如果视频帧，一般视频流的序号（stream_index）为0
//		++num;
//		if (packet && packet->stream_index == 0) {
//			if (Decode(g_inputContext->streams[0], packet.get(), videoFrame)) {
//				auto packetEncode = Encode(encodeContext, videoFrame);
//				if (packetEncode) {
//					ret = WriteFrame(packetEncode);
//					if (ret == 0 && num <=100000)
//						cout << "抓取成功:" << num<<endl;
//					else
//						break;
//				}
//			}
//		}
//	}
//
//	av_frame_free(&videoFrame);
//	avcodec_close(encodeContext);
//	return 0;
//}
//
//
//int main01(int argc, char* argv[]){
//	char filepath[1024] = "rtsp://admin:hikvision123@192.168.87.6/Streaming/Channels/1";
//	//const char* filepath = "C:\\Users\\admin\\Desktop\\input\\test.mp4";
//	const char* filename = "C:\\Users\\admin\\Desktop\\output\\test_output.jepg";
//	
//	CaptureJpeg(filepath, filename);
//
//    return 0;
//}

#include<iostream>
#include<memory>

extern "C" {
#include<libavformat/avformat.h>
#include<libavfilter/avfilter.h>
#include<libavutil/time.h>
#include<libavutil/frame.h>
}

static AVFormatContext* inputContext;
static AVFormatContext* outputContext;
static int64_t lastReadPacketTime = 0;

int interrupt_CB(void* ctx) {
	if ((av_gettime() - lastReadPacketTime) > 100 * 1000 * 1000) {
		std::cout << "主码流断开" << std::endl;
		return AVERROR_EOF;
	}
	return 0;
}

int OpenInputMeidaFile(const char* inputURL) {
	inputContext = avformat_alloc_context();
	lastReadPacketTime = av_gettime();
	inputContext->interrupt_callback.callback = interrupt_CB;

	AVInputFormat* fmt = nullptr;
	AVDictionary* options = nullptr;
	av_dict_set(&options, "rtsp_transport", "tcp", 0);
	av_dict_set(&options, "stimeout", "3000000", 0);


	int ret = avformat_open_input(&inputContext, inputURL, fmt, &options);
	if (ret != 0) {
		std::cout << "open inptut failed" << std::endl;
		return ret;
	}

	ret = avformat_find_stream_info(inputContext, nullptr);
	if (ret < 0) {
		std::cout << "get stream information failed" << std::endl;
		return ret;
	}
	return ret;
}

int OpenOutputMeidaFile(const char* outputURL) {
	int ret = avformat_alloc_output_context2(&outputContext, nullptr, "singleJpeg", outputURL);
	if (ret < 0) {
		std::cout << "open output failed" << std::endl;
		return ret;
	}
	ret = avio_open2(&outputContext->pb, outputURL, AVIO_FLAG_WRITE, nullptr, nullptr);
	if (ret < 0) {
		std::cout << "open failed" << std::endl;
		goto Error;
	}
	for (unsigned i = 0; i < inputContext->nb_streams; ++i) {
		if (inputContext->streams[i]->codec->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO)
			continue;
		AVStream* stream = avformat_new_stream(outputContext, inputContext->streams[i]->codec->codec);
		ret = avcodec_copy_context(stream->codec, inputContext->streams[i]->codec);
		if (ret < 0) {
			std::cout << "intput  stream copy into output failed " << std::endl;
			goto Error;
		}
	}
	ret = avformat_write_header(outputContext, nullptr);
	if (ret != 0) {
		std::cout << "write header into output file failed" << std::endl;
		goto Error;
	}
	return ret;
Error:
	if (outputContext) {
		for (unsigned i = 0; i < outputContext->nb_streams; ++i) {
			avcodec_close(outputContext->streams[i]->codec);
		}
		avformat_close_input(&inputContext);
	}
	return ret;
}

void CloseInput() {
	if (inputContext != nullptr)
		avformat_close_input(&inputContext);
}

void CloseOutput() {
	for (unsigned i = 0; i < outputContext->nb_streams; ++i) {
		avcodec_close(outputContext->streams[i]->codec);
	}
	avformat_close_input(&outputContext);
}

int InitDecodeContext(AVStream* inputStream) {
	auto codecId = inputStream->codec->codec_id;
	auto codec = avcodec_find_decoder(codecId);
	if (!codec) {
		std::cout << "find decodec failed" << std::endl;
		return -1;
	}
	return avcodec_open2(inputStream->codec, codec, nullptr);
}

int InitEncodeContext(AVStream* inputStream, AVCodecContext** encodeContext) {
	auto pitCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
	(*encodeContext) = avcodec_alloc_context3(pitCodec);
	(*encodeContext)->codec_id = pitCodec->id;
	(*encodeContext)->time_base.num = inputStream->codec->time_base.num;
	(*encodeContext)->time_base.den = inputStream->codec->time_base.den;
	(*encodeContext)->pix_fmt = *pitCodec->pix_fmts;
	(*encodeContext)->width = inputStream->codec->width;
	(*encodeContext)->height = inputStream->codec->height;
	return avcodec_open2((*encodeContext), pitCodec, nullptr);
}

std::shared_ptr<AVPacket> ReadPacketFrame() {
	std::shared_ptr<AVPacket>packet(static_cast<AVPacket*>(
		av_malloc(sizeof(AVPacket))), [&](AVPacket* p) {
			av_packet_free(&p);
			av_freep(&p);
		}
	);
	av_init_packet(packet.get());
	lastReadPacketTime = av_gettime();
	int ret = av_read_frame(inputContext, packet.get());
	if (ret != 0) {
		std::cout << "read frame into packet failed" << std::endl;
		return nullptr;
	}
	return packet;
}

bool Decode(AVStream* inputstream, AVPacket* packet, AVFrame* frame) {
	int gotPitcure_ptr = 0;
	auto ret = avcodec_decode_video2(inputstream->codec, frame, &gotPitcure_ptr, packet);
	if (ret < 0 || gotPitcure_ptr == 0) {
		std::cout << "decode failed" << std::endl;
		return false;
	}
	return true;
}

bool Decode_I(AVStream* inputstream, AVPacket* packet, AVFrame* frame) {
	int gotPitcure_ptr = 0;
	auto ret = avcodec_decode_video2(inputstream->codec, frame, &gotPitcure_ptr, packet);
	if (ret < 0 || gotPitcure_ptr == 0) {
		std::cout << "decode failed" << std::endl;
		return false;
	}
	if (frame->pict_type == AV_PICTURE_TYPE_I) {
		std::cout << "this is I picture" << std::endl;
	}
	else {
		std::cout << "this is BorP picture" << std::endl;
		return false;
	}
	return true;
}

bool Decode_B(AVStream* inputstream, AVPacket* packet, AVFrame* frame) {
	int gotPitcure_ptr = 0;
	auto ret = avcodec_decode_video2(inputstream->codec, frame, &gotPitcure_ptr, packet);
	if (ret < 0 || gotPitcure_ptr == 0) {
		std::cout << "decode failed" << std::endl;
		return false;
	}
	if (frame->pict_type == AV_PICTURE_TYPE_B) {
		std::cout << "this is B picture" << std::endl;
	}
	else {
		std::cout << "this is IorP picture" << std::endl;
		return false;
	}
	return true;
}

bool Decode_P(AVStream* inputstream, AVPacket* packet, AVFrame* frame) {
	int gotPitcure_ptr = 0;
	auto ret = avcodec_decode_video2(inputstream->codec, frame, &gotPitcure_ptr, packet);
	if (ret < 0 || gotPitcure_ptr == 0) {
		std::cout << "decode failed" << std::endl;
		return false;
	}
	if (frame->pict_type == AV_PICTURE_TYPE_P) {
		std::cout << "this is P picture" << std::endl;
	}
	else {
		std::cout << "this is IorB picture" << std::endl;
		return false;
	}
	return true;
}

std::shared_ptr<AVPacket> Encode(AVCodecContext* encodeContext, AVFrame* frame) {
	int gotPacket_ptr = 0;
	std::shared_ptr<AVPacket> pkt(static_cast<AVPacket*>(
		av_malloc(sizeof(AVPacket))), [&](AVPacket* p) {
			av_packet_free(&p);
			av_freep(&p);
		}
	);
	av_init_packet(pkt.get());
	pkt->data = nullptr;
	pkt->size = 0;
	int ret = avcodec_encode_video2(encodeContext, pkt.get(), frame, &gotPacket_ptr);
	if (ret != 0 || gotPacket_ptr <= 0) {
		std::cout << "encode failed" << std::endl;
		return nullptr;
	}
	/*if (frame->pict_type == AV_PICTURE_TYPE_B) {
		std::cout << "this is B picture" << std::endl;
	}
	else {
		std::cout << "this is IorP picture" << std::endl;
		return nullptr;
	}*/
	return pkt;
}

int WriteFrame(std::shared_ptr<AVPacket> packet) {
	auto inputStream = inputContext->streams[packet->stream_index];
	auto outputStream = outputContext->streams[packet->stream_index];
	AVPacket* pkt = packet.get();
	if (pkt->pts != AV_NOPTS_VALUE)
		pkt->pts = av_rescale_q(pkt->pts, inputStream->time_base, outputStream->time_base);
	if (pkt->dts != AV_NOPTS_VALUE)
		pkt->dts = av_rescale_q(pkt->dts, inputStream->time_base, outputStream->time_base);
	if (pkt->duration > 0)
		pkt->duration = av_rescale_q(pkt->duration, inputStream->time_base, outputStream->time_base);
	return av_interleaved_write_frame(outputContext, packet.get());
}

int main001() {

	av_register_all();
	avfilter_register_all();
	avformat_network_init();

	const char filepath[] = "rtsp://admin:hikvision123@192.168.87.6/Streaming/Channels/1";
	const char* filename = "C:\\Users\\admin\\Desktop\\output\\new_test_outputB.jepg";

	int ret = OpenInputMeidaFile(filepath);
	if (ret == 0)
		OpenOutputMeidaFile(filename);
	if (ret < 0) {
		std::cout << "ERROR" << std::endl;
		CloseInput();
		CloseOutput();
	}

	AVCodecContext* encodeContext = nullptr;
	InitDecodeContext(inputContext->streams[0]);
	InitEncodeContext(inputContext->streams[0], &encodeContext);
	AVFrame* videoFrame = av_frame_alloc();
	int getNum = 0;
	while (true) {
		auto videoPacket = ReadPacketFrame();
		++getNum;
		if (videoPacket && videoPacket->stream_index == 0) {
			if (Decode(inputContext->streams[0], videoPacket.get(), videoFrame)) {
				auto packetEncode = Encode(encodeContext, videoFrame);
				if (packetEncode) {
					ret = WriteFrame(packetEncode);
					if (ret == 0 && getNum <= 100)
						std::cout << "get pitcure sucessful" << std::endl;
					else
						break;
				}
			}
		}
		else if (getNum > 100) {
			std::cout << "Encode failed" << std::endl;
			break;
		}
	}
	ret = avio_open(&outputContext->pb, filename, AVIO_FLAG_READ_WRITE);

	av_frame_free(&videoFrame);
	avcodec_close(encodeContext);

	return 0;
}
