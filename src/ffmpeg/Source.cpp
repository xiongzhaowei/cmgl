//
// Created by 熊朝伟 on 2023-06-13.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

Packet::Packet() : _packet(av_packet_alloc()) {
}

Packet::~Packet() {
	av_packet_free(&_packet);
}

AVPacket* Packet::packet() const {
	return _packet;
}

void Packet::reset() {
	av_packet_unref(_packet);
}

FileSource::FileSource(Thread* thread) : _packet(new Packet), _thread(thread), _controller(StreamController<Packet>::sync([thread = RefPtr<Thread>(thread)]() { thread->runOnThread([]() {}); }, [thread = RefPtr<Thread>(thread)]() { thread->runOnThread([]() {}); })) {
}

bool FileSource::open(const std::string& filename) {
	return Error::verify(avformat_open_input(&_context, filename.c_str(), nullptr, nullptr), __FUNCSIG__, __LINE__);
}

void FileSource::close() {
	avformat_close_input(&_context);
}

bool FileSource::available() const {
	return !_controller->isPaused() && !_controller->isClosed();
}

bool FileSource::read() {
	if (Error::verify(av_read_frame(_context, _packet->packet()), __FUNCSIG__, __LINE__)) {
		_controller->add(_packet);
		_packet->reset();
		return true;
	}
	return false;
}

RefPtr<StreamSubscription<Packet>> FileSource::listen(RefPtr<Consumer<Packet>> consumer) {
	return _controller->stream()->listen(consumer);
}

FileSourceStream::FileSourceStream(AVStream* stream, AVCodecContext* context, Thread* thread) : _stream(stream), _context(context), _controller(StreamController<Frame>::sync([thread = RefPtr<Thread>(thread)]() { thread->runOnThread([]() {}); }, [thread = RefPtr<Thread>(thread)]() { thread->runOnThread([]() {}); })) {

}

FileSourceStream::~FileSourceStream() {
	if (_context) avcodec_free_context(&_context);
}

RefPtr<StreamSubscription<Frame>> FileSourceStream::listen(RefPtr<Consumer<Frame>> consumer) {
	return _controller->stream()->listen(consumer);
}

void FileSourceStream::add(RefPtr<Packet> packet) {
	if (packet == nullptr) return;
	if (packet->packet() == nullptr) return;

	if (_stream->index == packet->packet()->stream_index) {
		if (Error::verify(avcodec_send_packet(_context, packet->packet()), __FUNCSIG__, __LINE__)) {
			while (true) {
				int error = avcodec_receive_frame(_context, _frame->frame());
				if (error < 0) {
					if (error != AVERROR(EAGAIN) && error != AVERROR_EOF) {
						Error::report(error, __FUNCSIG__, __LINE__);
					}
					break;
				}
				_controller->add(_frame);
			}
		}
	}
}

void FileSourceStream::addError() {

}

void FileSourceStream::close() {

}

FileSourceStream* FileSourceStream::from(AVStream* stream, Thread* thread) {
	const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
	if (!codec) return nullptr;

	AVCodecContext* context = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(context, stream->codecpar);
	if (avcodec_open2(context, codec, NULL) < 0) return nullptr;

	return new FileSourceStream(stream, context, thread);
}

FileSourceStream* FileSourceStream::audio(FileSource* source) {
	if (source == nullptr) return nullptr;
	if (source->_context == nullptr) return nullptr;

	for (uint32_t i = 0; i < source->_context->nb_streams; i++) {
		AVStream* stream = source->_context->streams[i];
		if (stream && stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			return FileSourceStream::from(stream, source->_thread);
		}
	}
	return nullptr;
}

FileSourceStream* FileSourceStream::video(FileSource* source) {
	if (source == nullptr) return nullptr;
	if (source->_context == nullptr) return nullptr;

	for (uint32_t i = 0; i < source->_context->nb_streams; i++) {
		AVStream* stream = source->_context->streams[i];
		if (stream && stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			return FileSourceStream::from(stream, source->_thread);
		}
	}
	return nullptr;
}

AVStream* FileSourceStream::stream() const {
	return _stream;
}

bool FileSourceStream::available() const {
	return !_controller->isPaused() && !_controller->isClosed();
}

bool FileSourceStream::match(AVFormatContext* format, AVPacket* packet) const {
	return format->streams[packet->stream_index] == _stream;
}

bool FileSourceStream::decode(AVPacket* packet, RefPtr<Frame> frame) {
	if (avcodec_send_packet(_context, packet) < 0) return false;
	if (avcodec_receive_frame(_context, frame->frame()) < 0) return false;

	_controller->add(frame);
	return true;
}

FileTarget* FileTarget::from(const char* format, const char* filename) {
	AVFormatContext* context = nullptr;
	if (!Error::verify(avformat_alloc_output_context2(&context, NULL, format, filename), __FUNCSIG__, __LINE__)) {
		return nullptr;
	}
	if (!context) return nullptr;

	return new FileTarget(context);
}

FileTarget::FileTarget(AVFormatContext* context) : _context(context) {
}

FileTarget::~FileTarget() {
}

bool FileTarget::openFile(const std::string& filename) {
	if (_context->oformat->flags & AVFMT_NOFILE) return true;
	return Error::verify(avio_open(&_context->pb, filename.c_str(), AVIO_FLAG_WRITE), __FUNCSIG__, __LINE__);
}

bool FileTarget::closeFile() {
	if (_context->oformat->flags & AVFMT_NOFILE) return true;
	return Error::verify(avio_closep(&_context->pb), __FUNCSIG__, __LINE__);
}

bool FileTarget::writeHeader(AVDictionary* options) {
	return Error::verify(avformat_write_header(_context, &options), __FUNCSIG__, __LINE__);
}

bool FileTarget::writeTrailer() {
	return Error::verify(av_write_trailer(_context), __FUNCSIG__, __LINE__);
}

void FileTarget::close() {
	if (_context) {
		avformat_free_context(_context);
		_context = nullptr;
	}
}

void FileTarget::add(RefPtr<Packet> packet) {
	int error = av_interleaved_write_frame(_context, packet->packet());

	bool result = Error::verify(error, __FUNCSIG__, __LINE__);
	assert(result);
}

void FileTarget::addError() {

}

FileTargetStream::FileTargetStream(FileTarget* owner, AVStream* stream) : _owner(owner), _stream(stream), _context(nullptr) {

}

FileTargetStream* FileTargetStream::from(FileTarget* target, const AVCodec* codec) {
	AVStream* stream = avformat_new_stream(target->_context, codec);
	if (stream == nullptr) return nullptr;

	return new FileTargetStream(target, stream);
}

FileTargetStream* FileTargetStream::audio(FileTarget* target) {
	if (target == nullptr) return nullptr;
	if (target->_context == nullptr) return nullptr;
	if (target->_context->oformat == nullptr) return nullptr;

	const AVCodec* codec = avcodec_find_encoder(target->_context->oformat->audio_codec);
	if (codec == nullptr) return nullptr;

	return from(target, codec);
}

FileTargetStream* FileTargetStream::video(FileTarget* target) {
	if (target == nullptr) return nullptr;
	if (target->_context == nullptr) return nullptr;
	if (target->_context->oformat == nullptr) return nullptr;

	const AVCodec* codec = avcodec_find_encoder(target->_context->oformat->video_codec);
	if (codec == nullptr) return nullptr;

	return from(target, codec);
}

void FileTargetStream::add(RefPtr<Frame> frame) {
	if (frame == nullptr) return;
	if (frame->frame() == nullptr) return;

	if (!Error::verify(avcodec_send_frame(_context, frame->frame()), __FUNCSIG__, __LINE__)) {
		addError();
		return;
	}

	RefPtr<Packet> packet = new Packet;
	while (true) {
		int error = avcodec_receive_packet(_context, packet->packet());
		if (error < 0) {
			if (error != AVERROR(EAGAIN) && error != AVERROR_EOF) {
				Error::report(error, __FUNCSIG__, __LINE__);
			}
			break;
		}
		av_packet_rescale_ts(packet->packet(), _context->time_base, _stream->time_base);
		packet->packet()->stream_index = _stream->index;
		_owner->add(packet);
		packet->reset();
	}
}

void FileTargetStream::addError() {

}

void FileTargetStream::close() {

}

bool FileTargetStream::open(AVSampleFormat format, int32_t bit_rate, int32_t sample_rate, const AVChannelLayout& ch_layout, AVDictionary* options) {
	assert(_context == nullptr);

	const AVCodec* codec = avcodec_find_encoder(_owner->_context->oformat->audio_codec);
	if (codec == nullptr) return nullptr;

	assert(codec->type == AVMEDIA_TYPE_AUDIO);

	AVCodecContext* context = avcodec_alloc_context3(codec);
	if (context != nullptr) {
		context->bit_rate = bit_rate;
		context->time_base = AVRational{ 1, sample_rate };
		Error::verify(av_channel_layout_copy(&context->ch_layout, &ch_layout), __FUNCSIG__, __LINE__);
		
		if (codec->sample_fmts) {
			context->sample_fmt = codec->sample_fmts[0];
			for (int i = 0; codec->sample_fmts[i] != AV_SAMPLE_FMT_NONE; i++) {
				if (codec->sample_fmts[i] == format) {
					context->sample_fmt = format;
					break;
				}
			}
		} else {
			context->sample_fmt = format;
		}
		if (codec->supported_samplerates) {
			context->sample_rate = codec->supported_samplerates[0];
			for (int i = 0; codec->supported_samplerates[i]; i++) {
				if (codec->supported_samplerates[i] == sample_rate) {
					context->sample_rate = sample_rate;
					break;
				}
			}
		} else {
			context->sample_rate = sample_rate;
		}
		if (Error::verify(avcodec_open2(context, codec, &options), __FUNCSIG__, __LINE__)) {
			if (Error::verify(avcodec_parameters_from_context(_stream->codecpar, context), __FUNCSIG__, __LINE__)) {
				_context = context;
				return true;
			}
		}

		avcodec_free_context(&context);
	}
	return false;
}

bool FileTargetStream::open(AVPixelFormat format, int32_t bit_rate, int32_t frame_rate, int32_t width, int32_t height, int32_t gop_size, int32_t max_b_frames, AVDictionary* options) {
	assert(_context == nullptr);

	const AVCodec* codec = avcodec_find_encoder(_owner->_context->oformat->video_codec);
	if (codec == nullptr) return nullptr;

	assert(codec->type == AVMEDIA_TYPE_VIDEO);

	AVCodecContext* context = avcodec_alloc_context3(codec);
	if (context != nullptr) {
		context->bit_rate = bit_rate;
		context->width = width;
		context->height = height;
		context->gop_size = gop_size;
		context->max_b_frames = max_b_frames;
		
		if (codec->pix_fmts) {
			context->pix_fmt = codec->pix_fmts[0];
			for (int i = 0; codec->pix_fmts[i] != AV_PIX_FMT_NONE; i++) {
				if (codec->pix_fmts[i] == format) {
					context->pix_fmt = format;
					break;
				}
			}
		} else {
			context->pix_fmt = format;
		}
		if (codec->supported_framerates) {
			context->framerate = codec->supported_framerates[0];
			for (int i = 0; codec->supported_framerates[i].num || codec->supported_framerates[i].den; i++) {
				if (codec->supported_framerates[i].num / codec->supported_framerates[i].den == frame_rate) {
					context->framerate = codec->supported_framerates[i];
					break;
				}
			}
		} else {
			context->framerate = AVRational{ frame_rate, 1 };
		}
		context->time_base = AVRational{ context->framerate.den, context->framerate.num };

		if (Error::verify(avcodec_open2(context, codec, &options), __FUNCSIG__, __LINE__)) {
			if (Error::verify(avcodec_parameters_from_context(_stream->codecpar, context), __FUNCSIG__, __LINE__)) {
				_context = context;
				return true;
			}
		}

		avcodec_free_context(&context);
	}
	return false;
}
