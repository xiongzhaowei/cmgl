﻿//
// Created by 熊朝伟 on 2023-06-13.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

MovieSource::MovieSource() : _packet(new Packet), _controller(StreamController<Packet>::sync()) {
}

bool MovieSource::open(RefPtr<MovieFile> file) {
	AVFormatContext* context = avformat_alloc_context();
	context->pb = avio_alloc_context(
		file->buffer(),
		file->bufferSize(),
		0,
		file.value(),
		[](void *opaque, uint8_t *buf, int buf_size) { return static_cast<MovieFile*>(opaque)->read(buf, buf_size); },
		nullptr,
		[](void *opaque, int64_t offset, int whence) { return static_cast<MovieFile*>(opaque)->seek(offset, whence); }
	);

	if (Error::verify(avformat_open_input(&context, nullptr, nullptr, nullptr), __FUNCSIG__, __LINE__)) {
		if (Error::verify(avformat_find_stream_info(context, nullptr), __FUNCSIG__, __LINE__)) {
			_context = context;
			return true;
		}
		avformat_close_input(&context);
	}
	return false;
}

bool MovieSource::open(const std::string& filename) {
	AVFormatContext* context = nullptr;

	if (Error::verify(avformat_open_input(&context, filename.c_str(), nullptr, nullptr), __FUNCSIG__, __LINE__)) {
		if (Error::verify(avformat_find_stream_info(context, nullptr), __FUNCSIG__, __LINE__)) {
			_context = context;
			return true;
		}
		avformat_close_input(&context);
	}
	return false;
}

void MovieSource::close() {
	avformat_close_input(&_context);
}

AVFormatContext* MovieSource::context() const {
	return _context;
}

AVStream* MovieSource::stream(AVMediaType codecType) const {
	for (uint32_t i = 0; i < _context->nb_streams; i++) {
		AVStream* stream = _context->streams[i];
		if (stream && stream->codecpar->codec_type == codecType) {
			return stream;
		}
	}
	return nullptr;
}

bool MovieSource::available() const {
	return _controller->available();
}

bool MovieSource::read() {
	if (Error::verify(av_read_frame(_context, _packet->packet()), __FUNCSIG__, __LINE__)) {
		_controller->add(_packet);
		_packet->reset();
		return true;
	}
	return false;
}

bool MovieSource::seek(double time) {
	if (_context == nullptr) return false;

	int64_t ts = std::clamp<int64_t>(int64_t(time * AV_TIME_BASE), 0, _context->duration);
	bool result = Error::verify(avformat_seek_file(_context, -1, 0, ts, _context->duration, 0), __FUNCSIG__, __LINE__);
	if (_context->pb) avio_flush(_context->pb);
	avformat_flush(_context);
	return result;
}

void MovieSource::skip(const std::function<bool(AVPacket*)>& skipWhere) {
	while (Error::verify(av_read_frame(_context, _packet->packet()), __FUNCSIG__, __LINE__)) {
		_controller->add(_packet);
		bool result = skipWhere(_packet->packet());
		_packet->reset();
		if (result) break;
	}
}

RefPtr<StreamSubscription> MovieSource::listen(RefPtr<StreamConsumer<Packet>> consumer) {
	return _controller->stream()->listen(consumer);
}

MovieDecoder::MovieDecoder(RefPtr<StreamConsumer<Frame>> output, AVStream* stream, AVCodecContext* context) : _output(output), _stream(stream), _context(context), _frame(Frame::alloc()) {
	assert(context != nullptr);
}

MovieDecoder::~MovieDecoder() {
	assert(_context != nullptr);
	avcodec_free_context(&_context);
}

void MovieDecoder::add(RefPtr<Packet> packet) {
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
				_output->add(_frame);
			}
		}
	}
}

void MovieDecoder::addError() {
	_output->addError();
}

void MovieDecoder::close() {
	_output->close();
}

bool MovieDecoder::available() const {
	return _output->available();
}

AVStream* MovieDecoder::stream() const {
	return _stream;
}

AVCodecContext* MovieDecoder::context() const {
	return _context;
}

RefPtr<MovieDecoder> MovieDecoder::from(RefPtr<StreamConsumer<Frame>> output, AVStream* stream, AVDictionary* options) {
	const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
	if (!codec) return nullptr;

	AVCodecContext* context = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(context, stream->codecpar);
	if (avcodec_open2(context, codec, &options) < 0) return nullptr;

	return new MovieDecoder(output, stream, context);
}

MovieSourceStream::MovieSourceStream(RefPtr<StreamController<Frame>> controller, RefPtr<MovieDecoder> decoder, RefPtr<StreamSubscription> subscription) : _decoder(decoder), _controller(controller), _subscription(subscription) {

}

MovieSourceStream::~MovieSourceStream() {
	if (_subscription != nullptr) {
		_subscription->cancel();
		_subscription = nullptr;
	}
}

RefPtr<StreamSubscription> MovieSourceStream::listen(RefPtr<StreamConsumer<Frame>> consumer) {
	return _controller->stream()->listen(consumer);
}

AVStream* MovieSourceStream::stream() const {
	return _decoder->stream();
}

AVCodecContext* MovieSourceStream::context() const {
	return _decoder->context();
}

bool MovieSourceStream::available() const {
	return _controller->available();
}

RefPtr<Stream<Frame>> MovieSourceStream::convert(AVSampleFormat sample_fmt, AVChannelLayout ch_layout, int32_t sample_rate) {
	assert(stream()->codecpar->codec_type == AVMEDIA_TYPE_AUDIO);
	return Stream<Frame>::convert(AudioConverter::from(ch_layout, sample_fmt, sample_rate, _decoder->context()->ch_layout, _decoder->context()->sample_fmt, _decoder->context()->sample_rate));
}

RefPtr<Stream<Frame>> MovieSourceStream::convert(AVSampleFormat sample_fmt) {
	assert(stream()->codecpar->codec_type == AVMEDIA_TYPE_AUDIO);
	return Stream<Frame>::convert(AudioConverter::from(_decoder->context()->ch_layout, sample_fmt, _decoder->context()->sample_rate, _decoder->context()->ch_layout, _decoder->context()->sample_fmt, _decoder->context()->sample_rate));
}

RefPtr<Stream<Frame>> MovieSourceStream::convert(AVPixelFormat format) {
	assert(stream()->codecpar->codec_type == AVMEDIA_TYPE_VIDEO);
	if (stream()->codecpar->format == format) return this;
	return Stream<Frame>::convert(VideoConverter::create(stream()->codecpar, format));
}

RefPtr<MovieSourceStream> MovieSourceStream::from(RefPtr<MovieSource> source, AVStream* stream, AVDictionary* options) {
	const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
	if (!codec) return nullptr;

	AVCodecContext* context = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(context, stream->codecpar);
	if (avcodec_open2(context, codec, NULL) < 0) return nullptr;

	RefPtr<StreamController<Frame>> controller = StreamController<Frame>::sync();
	RefPtr<MovieDecoder> decoder = MovieDecoder::from(controller, stream, options);
	RefPtr<MovieSourceStream> result = new MovieSourceStream(controller, decoder, source->listen(decoder));
	return result;
}

RefPtr<MovieSourceStream> MovieSourceStream::audio(RefPtr<MovieSource> source, AVDictionary* options) {
	AVStream* stream = source->stream(AVMEDIA_TYPE_AUDIO);
	if (stream == nullptr) return nullptr;
	return from(source, stream, options);
}

RefPtr<MovieSourceStream> MovieSourceStream::video(RefPtr<MovieSource> source, AVDictionary* options) {
	AVStream* stream = source->stream(AVMEDIA_TYPE_VIDEO);
	if (stream == nullptr) return nullptr;
	return from(source, stream, options);
}

MovieBufferedConsumer::MovieBufferedConsumer(RefPtr<Stream<Frame>> stream, uint32_t maxCount) : _subscription(stream->listen(this)), _maxCount(maxCount) {
}

void MovieBufferedConsumer::add(RefPtr<Frame> frame) {
    if (frame == nullptr) return;

    RefPtr<Frame> output;
    if (_converter) {
		output = _converter->convert(frame);
    } else {
		output = Frame::alloc();
		output->swap(frame);
    }
    push(output);
}

void MovieBufferedConsumer::addError() {

}

void MovieBufferedConsumer::close() {
	if (_subscription != nullptr) {
		_subscription->cancel();
		_subscription = nullptr;
	}
}

bool MovieBufferedConsumer::available() const {
	return size() < _maxCount;
}

size_t MovieBufferedConsumer::size() const {
	std::lock_guard<std::mutex> lock(_mutex);
	return _list.size();
}

int64_t MovieBufferedConsumer::timestamp() const {
	std::lock_guard<std::mutex> lock(_mutex);
	return _list.empty() ? -1 : _list.front()->timestamp();
}

void MovieBufferedConsumer::push(RefPtr<Frame> frame) {
	std::lock_guard<std::mutex> lock(_mutex);
	auto pred = [](RefPtr<Frame> left, RefPtr<Frame> right) {
		return left->timestamp() < right->timestamp();
	};
	_list.insert(std::lower_bound(_list.begin(), _list.end(), frame, pred), frame);
}

RefPtr<Frame> MovieBufferedConsumer::pop() {
	std::lock_guard<std::mutex> lock(_mutex);
	if (_list.empty()) return nullptr;

	RefPtr<Frame> frame = _list.front();
	_list.pop_front();
	return frame;
}

RefPtr<Frame> MovieBufferedConsumer::pop(int64_t timestamp) {
	std::lock_guard<std::mutex> lock(_mutex);
	if (_list.empty()) return nullptr;
	if (_list.front()->timestamp() > timestamp) return nullptr;

	RefPtr<Frame> frame = _list.front();
	_list.pop_front();
	return frame;
}

void MovieBufferedConsumer::clear() {
	std::lock_guard<std::mutex> lock(_mutex);
	_list.clear();
}

MovieEncoder::MovieEncoder(RefPtr<StreamConsumer<Packet>> output, AVStream* stream, AVCodecContext* context) : _output(output), _stream(stream), _context(context), _packet(new Packet) {

}

MovieEncoder::~MovieEncoder() {
	avcodec_free_context(&_context);
}

void MovieEncoder::add(RefPtr<Frame> frame) {
	if (frame == nullptr) return;
	if (frame->frame() == nullptr) return;

	if (!Error::verify(avcodec_send_frame(_context, frame->frame()), __FUNCSIG__, __LINE__)) {
		addError();
		return;
	}

	while (true) {
		int error = avcodec_receive_packet(_context, _packet->packet());
		if (error < 0) {
			if (error != AVERROR(EAGAIN) && error != AVERROR_EOF) {
				Error::report(error, __FUNCSIG__, __LINE__);
			}
			addError();
			break;
		}
		av_packet_rescale_ts(_packet->packet(), context()->time_base, _stream->time_base);
		_packet->packet()->stream_index = _stream->index;
		_output->add(_packet);
		_packet->reset();
	}
}

void MovieEncoder::addError() {
	_output->addError();
}

void MovieEncoder::close() {
	_output->close();
}

bool MovieEncoder::available() const {
	return _output->available();
}

AVStream* MovieEncoder::stream() const {
	return _stream;
}

AVCodecContext* MovieEncoder::context() const {
	return _context;
}

RefPtr<MovieEncoder> MovieEncoder::audio(RefPtr<StreamConsumer<Packet>> output, const AVCodec* codec, AVStream* stream, int32_t bit_rate, AVSampleFormat format, int32_t sample_rate, AVChannelLayout ch_layout, AVDictionary* options) {
	if (nullptr == stream || nullptr == stream->codecpar) return nullptr;
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
			if (Error::verify(avcodec_parameters_from_context(stream->codecpar, context), __FUNCSIG__, __LINE__)) {
				return new MovieEncoder(output, stream, context);
			}
		}
		avcodec_free_context(&context);
	}
	return nullptr;
}

RefPtr<MovieEncoder> MovieEncoder::video(RefPtr<StreamConsumer<Packet>> output, const AVCodec* codec, AVStream* stream, int32_t bit_rate, AVPixelFormat format, int32_t frame_rate, int32_t width, int32_t height, int32_t gop_size, int32_t max_b_frames, AVDictionary* options) {
	if (nullptr == stream || nullptr == stream->codecpar) return nullptr;
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
			if (Error::verify(avcodec_parameters_from_context(stream->codecpar, context), __FUNCSIG__, __LINE__)) {
				return new MovieEncoder(output, stream, context);
			}
		}
		avcodec_free_context(&context);
	}
	return nullptr;
}

MovieTarget::MovieTarget(AVFormatContext* context) : _context(context) {

}

MovieTarget::~MovieTarget() {
	if (_context) {
		avformat_free_context(_context);
		_context = nullptr;
	}
}

void MovieTarget::add(RefPtr<Packet> packet) {
	bool result = Error::verify(av_interleaved_write_frame(_context, packet->packet()), __FUNCSIG__, __LINE__);
	assert(result);
}

void MovieTarget::addError() {

}

void MovieTarget::close() {

}

bool MovieTarget::available() const {
	return true;
}

AVFormatContext* MovieTarget::context() const {
	return _context;
}

RefPtr<MovieEncoder> MovieTarget::encoder(int32_t bit_rate, AVSampleFormat format, int32_t sample_rate, AVChannelLayout ch_layout, AVDictionary* options) {
	if (_context->oformat == nullptr) return nullptr;

	const AVCodec* codec = avcodec_find_encoder(_context->oformat->audio_codec);
	if (codec == nullptr) return nullptr;

	AVStream* stream = avformat_new_stream(_context, codec);
	if (stream == nullptr) return nullptr;

	return MovieEncoder::audio(this, codec, stream, bit_rate, format, sample_rate, ch_layout, options);
}

RefPtr<MovieEncoder> MovieTarget::encoder(int32_t bit_rate, AVPixelFormat format, int32_t frame_rate, int32_t width, int32_t height, int32_t gop_size, int32_t max_b_frames, AVDictionary* options) {
	if (_context->oformat == nullptr) return nullptr;

	const AVCodec* codec = avcodec_find_encoder(_context->oformat->video_codec);
	if (codec == nullptr) return nullptr;

	AVStream* stream = avformat_new_stream(_context, codec);
	if (stream == nullptr) return nullptr;

	return MovieEncoder::video(this, codec, stream, bit_rate, format, frame_rate, width, height, gop_size, max_b_frames, options);
}

bool MovieTarget::openFile(const std::string& filename) {
	if (_context->oformat->flags & AVFMT_NOFILE) return true;
	return Error::verify(avio_open(&_context->pb, filename.c_str(), AVIO_FLAG_WRITE), __FUNCSIG__, __LINE__);
}

bool MovieTarget::closeFile() {
	if (_context->oformat->flags & AVFMT_NOFILE) return true;
	return Error::verify(avio_closep(&_context->pb), __FUNCSIG__, __LINE__);
}

bool MovieTarget::writeHeader(AVDictionary* options) {
	return Error::verify(avformat_write_header(_context, &options), __FUNCSIG__, __LINE__);
}

bool MovieTarget::writeTrailer() {
	return Error::verify(av_write_trailer(_context), __FUNCSIG__, __LINE__);
}

MovieTarget* MovieTarget::from(const char* format, const char* filename) {
	AVFormatContext* context = nullptr;
	if (!Error::verify(avformat_alloc_output_context2(&context, NULL, format, filename), __FUNCSIG__, __LINE__)) {
		return nullptr;
	}
	if (!context) return nullptr;

	return new MovieTarget(context);
}
