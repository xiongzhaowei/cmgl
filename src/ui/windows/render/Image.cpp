#include "defines.h"

OMP_UI_WINDOWS_USING_NAMESPACE

ImageDataSource::ImageDataSource(const std::function<RefPtr<ImageData>()>& loader) : _loader(loader) {
}

void ImageDataSource::load(RefPtr<render::RenderContext> context) {
	if (!_loader) return;
	RefPtr<ImageData> image = _loader();
	if (!image) return;

	image->load([this, context](int32_t width, int32_t height, int32_t stride, void* bytes) {
		_texture = context->createTexture();
		_texture->setImage(width, height, (uint8_t*)bytes, render::Texture::RGBA);
		_size = glm::vec2(width, height);
	});
}

void ImageDataSource::unload(RefPtr<render::RenderContext> context) {
	_texture = nullptr;
}

glm::vec2 ImageDataSource::size() const {
	return _size;
}

void ImageDataSource::draw(
	RefPtr<render::RenderContext> context,
	RefPtr<render::Framebuffer> framebuffer,
	const glm::mat4& globalMatrix,
	const glm::mat4& localMatrix,
	const glm::mat4& clipMatrix,
	const glm::vec2& size,
	float alpha
) {
	if (_texture == nullptr) load(context);

	context->draw(
		render::Renderer::RGBA,
		framebuffer,
		globalMatrix,
		localMatrix,
		clipMatrix,
		render::Renderer::kColorConversionExchangeRedAndBlue,
		size,
		alpha,
		_texture->data()
	);
}

windows::Image::Image(const std::function<RefPtr<ImageData>()>& loader) : _loader(loader) {
	RefPtr<ImageData> imageData = _loader();
	if (imageData) {
		_size = glm::vec2(imageData->width(), imageData->height());
	}
}

float windows::Image::width() const {
	return _size.x;
}

float windows::Image::height() const {
	return _size.y;
}

RefPtr<render::DataSource> windows::Image::renderObject() {
	if (_renderObject) return _renderObject.value();

	RefPtr<ImageDataSource> renderObject = new ImageDataSource(_loader);
	_renderObject = renderObject;
	return renderObject;
}
