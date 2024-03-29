
#if defined(GDIPVER)
static_assert(GDIPVER == 0x0110)
#else
#define GDIPVER 0x0110
#endif

#include <Windows.h>
#include <gdiplus.h>
#include "defines.h"

#pragma comment(lib, "gdiplus.lib")

OMP_UI_WINDOWS_USING_NAMESPACE
using namespace gdiplus;

WeakPtr<Token> Token::shared;

Token::Token() {
	::Gdiplus::GdiplusStartup(&m_hToken, &m_input, NULL);
}

Token::~Token() {
	::Gdiplus::GdiplusShutdown(m_hToken);
}

RefPtr<Token> Token::Get() {
	if (shared == nullptr) {
		shared = new Token;
	}
	return shared;
}

gdiplus::ImageData::ImageData(Gdiplus::Bitmap* pImage) : m_pImage(pImage) {
	assert(pImage != nullptr);
}

gdiplus::ImageData::~ImageData() {
	if (m_pImage) {
		delete m_pImage;
		m_pImage = nullptr;
	}
}

void gdiplus::ImageData::load(const std::function<void(int32_t width, int32_t height, int32_t stride, void* bytes)>& callback) {
	if (m_pImage) {
		Gdiplus::Rect rect(0, 0, width(), height());
		Gdiplus::BitmapData bitmapData = { 0 };
		if (m_pImage->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppPARGB, &bitmapData) == Gdiplus::Status::Ok) {
			callback(bitmapData.Width, bitmapData.Height, bitmapData.Stride, bitmapData.Scan0);
			m_pImage->UnlockBits(&bitmapData);
		}
	}
}

gdiplus::Image::Image(Gdiplus::Bitmap* pImage) : m_pImage(pImage) {
	assert(pImage != nullptr);
}

gdiplus::Image::~Image() {
	if (m_pImage) {
		delete m_pImage;
		m_pImage = nullptr;
	}
}

RefPtr<gdiplus::Image> gdiplus::Image::FromFile(LPCWSTR lpszPath) {
	RefPtr<Token> spToken = Token::Get();
	return new Image(Gdiplus::Bitmap::FromFile(lpszPath));
}

RefPtr<gdiplus::Image> gdiplus::Image::FromHICON(HICON hIcon) {
	RefPtr<Token> spToken = Token::Get();
	return new Image(Gdiplus::Bitmap::FromHICON(hIcon));
}

Path::Path(Gdiplus::GraphicsPath* pPath) : m_pPath(pPath) {

}

Path::~Path() {
	if (m_pPath) {
		delete m_pPath;
		m_pPath = nullptr;
	}
}

Region::Region(Gdiplus::Region* pRegion) : m_pRegion(pRegion) {

}

Region::~Region() {
	if (m_pRegion) {
		delete m_pRegion;
		m_pRegion = nullptr;
	}
}

RefPtr<Region> Region::CreateRoundRectRegion(Rect rect, float radius) {
	return new Region(new Gdiplus::Region(CreateRoundRectRgn(
		(int)roundf(rect.origin.x),
		(int)roundf(rect.origin.y),
		(int)roundf(rect.origin.x + rect.size.width),
		(int)roundf(rect.origin.y + rect.size.height),
		(int)roundf(radius * 2),
		(int)roundf(radius * 2)
	)));
}

Graphics::Graphics(Gdiplus::Graphics* pGraphics) : m_pGraphics(pGraphics) {
	assert(pGraphics != nullptr);
}

Graphics::~Graphics() {
	if (m_pGraphics) {
		delete m_pGraphics;
		m_pGraphics = nullptr;
	}
}

RefPtr<Graphics> Graphics::FromImage(Image* image) {
	RefPtr<Token> spToken = Token::Get();
	return new Graphics(Gdiplus::Graphics::FromImage(image->m_pImage));
}

RefPtr<Graphics> Graphics::FromWindow(HWND hWnd) {
	RefPtr<Token> spToken = Token::Get();
	return new Graphics(Gdiplus::Graphics::FromHWND(hWnd));
}

RefPtr<Graphics> Graphics::FromHDC(HDC hDC) {
	RefPtr<Token> spToken = Token::Get();
	return new Graphics(Gdiplus::Graphics::FromHDC(hDC));
}

static void DetermineBoxes(double sigma, int* boxes, int count) {
	int lower = (int)floor(sqrt((12 * sigma * sigma / count) + 1));
	if (lower % 2 == 0) lower--;
	int upper = lower + 2;
	int median = (int)round((12 * sigma * sigma - count * lower * lower - 4 * count * lower - 3 * count) / (-4 * lower - 4));

	for (int i = 0; i < count; i++) {
		boxes[i] = (i < median) ? lower : upper;
	}
}

static void BoxBlur(Gdiplus::BitmapData* data1, Gdiplus::BitmapData* data2, int width, int height, int radius) {
	constexpr int CHANNELS = 4;

	uint8_t* p1 = (uint8_t*)(void*)data1->Scan0;
	uint8_t* p2 = (uint8_t*)(void*)data2->Scan0;

	int radius2 = 2 * radius + 1;
	int sum[CHANNELS];
	int FirstValue[CHANNELS];
	int LastValue[CHANNELS];

	// Horizontal
	int stride = data1->Stride;
	for (int row = 0; row < height; row++) {
		int start = row * stride;
		int left = start;
		int right = start + radius * CHANNELS;

		for (int channel = 0; channel < CHANNELS; channel++) {
			FirstValue[channel] = p1[start + channel];
			LastValue[channel] = p1[start + (width - 1) * CHANNELS + channel];
			sum[channel] = (radius + 1) * FirstValue[channel];
		}
		for (int column = 0; column < radius; column++) {
			for (int channel = 0; channel < CHANNELS; channel++) {
				sum[channel] += p1[start + column * CHANNELS + channel];
			}
		}
		for (int column = 0; column <= radius; column++, right += CHANNELS, start += CHANNELS) {
			for (int channel = 0; channel < CHANNELS; channel++) {
				sum[channel] += p1[right + channel] - FirstValue[channel];
				p2[start + channel] = (uint8_t)(sum[channel] / radius2);
			}
		}
		for (int column = radius + 1; column < width - radius; column++, left += CHANNELS, right += CHANNELS, start += CHANNELS) {
			for (int channel = 0; channel < CHANNELS; channel++) {
				sum[channel] += p1[right + channel] - p1[left + channel];
				p2[start + channel] = (uint8_t)(sum[channel] / radius2);
			}
		}
		for (int column = width - radius; column < width; column++, left += CHANNELS, start += CHANNELS) {
			for (int channel = 0; channel < CHANNELS; channel++) {
				sum[channel] += LastValue[channel] - p1[left + channel];
				p2[start + channel] = (uint8_t)(sum[channel] / radius2);
			}
		}
	}

	// Vertical
	stride = data2->Stride;
	for (int column = 0; column < width; column++) {
		int start = column * CHANNELS;
		int top = start;
		int bottom = start + radius * stride;

		for (int channel = 0; channel < CHANNELS; channel++) {
			FirstValue[channel] = p2[start + channel];
			LastValue[channel] = p2[start + (height - 1) * stride + channel];
			sum[channel] = (radius + 1) * FirstValue[channel];
		}
		for (int row = 0; row < radius; row++) {
			for (int channel = 0; channel < CHANNELS; channel++) {
				sum[channel] += p2[start + row * stride + channel];
			}
		}
		for (int row = 0; row <= radius; row++, bottom += stride, start += stride) {
			for (int channel = 0; channel < CHANNELS; channel++) {
				sum[channel] += p2[bottom + channel] - FirstValue[channel];
				p1[start + channel] = (uint8_t)(sum[channel] / radius2);
			}
		}
		for (int row = radius + 1; row < height - radius; row++, top += stride, bottom += stride, start += stride) {
			for (int channel = 0; channel < CHANNELS; channel++) {
				sum[channel] += p2[bottom + channel] - p2[top + channel];
				p1[start + channel] = (uint8_t)(sum[channel] / radius2);
			}
		}
		for (int row = height - radius; row < height; row++, top += stride, start += stride) {
			for (int channel = 0; channel < CHANNELS; channel++) {
				sum[channel] += LastValue[channel] - p2[top + channel];
				p1[start + channel] = (uint8_t)(sum[channel] / radius2);
			}
		}
	}
}

static Gdiplus::Bitmap* CreateShadow(Gdiplus::Bitmap* bitmap, int radius, float opacity) {
	// Alpha mask with opacity
	Gdiplus::ColorMatrix matrix = { {
		{  0.0f,  0.0f,  0.0f, 0.0f,	0.0f },
		{  0.0f,  0.0f,  0.0f, 0.0f,	0.0f },
		{  0.0f,  0.0f,  0.0f, 0.0f,	0.0f },
		{ -1.0f, -1.0f, -1.0f, opacity, 0.0f },
		{  1.0f,  1.0f,  1.0f, 0.0f,	1.0f }
	} };

	Gdiplus::ImageAttributes* imageAttributes = new Gdiplus::ImageAttributes;
	imageAttributes->SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
	Gdiplus::Bitmap* shadow = new Gdiplus::Bitmap(bitmap->GetWidth() + 4 * radius, bitmap->GetHeight() + 4 * radius);
	Gdiplus::Graphics* graphics = Gdiplus::Graphics::FromImage(shadow);
	graphics->DrawImage(bitmap, Gdiplus::Rect(2 * radius, 2 * radius, bitmap->GetWidth(), bitmap->GetHeight()), 0, 0, bitmap->GetWidth(), bitmap->GetHeight(), Gdiplus::UnitPixel, imageAttributes);
	delete imageAttributes;
	delete graphics;

	// Gaussian blur
	Gdiplus::Bitmap* clone = shadow->Clone(0, 0, shadow->GetWidth(), shadow->GetHeight(), PixelFormat32bppARGB);
	Gdiplus::BitmapData shadowData;
	Gdiplus::BitmapData cloneData;
	Gdiplus::Rect shadowRect(0, 0, shadow->GetWidth(), shadow->GetHeight());
	Gdiplus::Rect cloneRect(0, 0, clone->GetWidth(), clone->GetHeight());
	shadow->LockBits(&shadowRect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &shadowData);
	clone->LockBits(&cloneRect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &cloneData);

	int boxes[3] = { 0 };
	DetermineBoxes(radius, boxes, 3);

	BoxBlur(&shadowData, &cloneData, shadow->GetWidth(), shadow->GetHeight(), (boxes[0] - 1) / 2);
	BoxBlur(&shadowData, &cloneData, shadow->GetWidth(), shadow->GetHeight(), (boxes[1] - 1) / 2);
	BoxBlur(&shadowData, &cloneData, shadow->GetWidth(), shadow->GetHeight(), (boxes[2] - 1) / 2);

	shadow->UnlockBits(&shadowData);
	clone->UnlockBits(&cloneData);

	delete clone;

	return shadow;
}

RefPtr<gdiplus::Image> gdiplus::Image::Create(int width, int height) {
	return new Image(new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB));
}

RefPtr<gdiplus::Image> gdiplus::Image::CreateShadow(RefPtr<Image> image, int radius, float opacity) {
	return new Image(::CreateShadow(image->m_pImage, radius, opacity));
}

bool Graphics::DrawImage(Image* image, Gdiplus::RectF source, Gdiplus::RectF target, Matrix* transform) {
	if (nullptr == m_pGraphics) return false;
	if (image == nullptr) return false;

	// 不知为何，这里需要减1，否则会多绘制1像素的空白区域，当图片拉伸时，会导致很明显的画面问题
	source.Width -= 1;
	source.Height -= 1;
	target.Width -= 1;
	target.Height -= 1;

	if (transform) {
		m_pGraphics->SetTransform(transform->m_pMatrix);
		Gdiplus::Status status = m_pGraphics->DrawImage(image->m_pImage, target, source, Gdiplus::Unit::UnitPixel);
		m_pGraphics->ResetTransform();
		return status == Gdiplus::Status::Ok;
	} else {
		return m_pGraphics->DrawImage(image->m_pImage, target, source, Gdiplus::Unit::UnitPixel) == Gdiplus::Status::Ok;
	}
}

bool Graphics::DrawString(const std::wstring& text, Gdiplus::Font* font, COLORREF color, TextAlignment textAlignment, TextVerticalAlignment textVerticalAlignment, Gdiplus::RectF target, Matrix* transform) {
	if (nullptr == m_pGraphics) return false;

	Gdiplus::StringFormat format(Gdiplus::StringFormat::GenericDefault());
	switch (textAlignment) {
	case TextAlignment::left:
		format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
		break;
	case TextAlignment::center:
		format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);
		break;
	case TextAlignment::right:
		format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentFar);
		break;
	}
	switch (textVerticalAlignment) {
	case TextVerticalAlignment::top:
		format.SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
		break;
	case TextVerticalAlignment::center:
		format.SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);
		break;
	case TextVerticalAlignment::bottom:
		format.SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentFar);
		break;
	}

	Gdiplus::SolidBrush brush = Gdiplus::SolidBrush(Gdiplus::Color(color));
	if (transform) {
		m_pGraphics->SetTransform(transform->m_pMatrix);
		Gdiplus::Status status = m_pGraphics->DrawString(text.c_str(), (int32_t)text.size(), font, target, &format, &brush);
		m_pGraphics->ResetTransform();
		return status == Gdiplus::Status::Ok;
	} else {
		return m_pGraphics->DrawString(text.c_str(), (int32_t)text.size(), font, target, &format, &brush) == Gdiplus::Status::Ok;
	}
}

bool Graphics::DrawRect(COLORREF color, Gdiplus::RectF target, Matrix* transform) {
	if (nullptr == m_pGraphics) return false;

	Gdiplus::Pen pen((Gdiplus::Color(color)));
	if (transform) {
		m_pGraphics->SetTransform(transform->m_pMatrix);
		Gdiplus::Status status = m_pGraphics->DrawRectangle(&pen, target);
		m_pGraphics->ResetTransform();
		return status == Gdiplus::Status::Ok;
	} else {
		return m_pGraphics->DrawRectangle(&pen, target) == Gdiplus::Status::Ok;
	}
}

bool Graphics::Clear(COLORREF color) {
	return m_pGraphics->Clear(Gdiplus::Color(color)) == Gdiplus::Status::Ok;
}

void Graphics::SetClip(Region* region) {
	if (region) {
		m_pGraphics->SetClip(region->m_pRegion);
	} else {
		m_pGraphics->ResetClip();
	}
}

Framebuffer::Framebuffer(uint16_t nWidth, uint16_t nHeight) : m_nWidth(nWidth), m_nHeight(nHeight) {
	RefPtr<Token> spToken = Token::Get();
	Gdiplus::Bitmap* pImage = new Gdiplus::Bitmap(nWidth, nHeight, PixelFormat32bppPARGB);
	m_spImage = new Image(pImage);
	m_spGraphics = new Graphics(Gdiplus::Graphics::FromImage(pImage));
}

RefPtr<StaticLayer> StaticLayer::Create(uint16_t nWidth, uint16_t nHeight) {
	RefPtr<StaticLayer> layer = new StaticLayer;
	layer->m_spFramebuffer = new Framebuffer(nWidth, nHeight);
	layer->m_fWidth = nWidth;
	layer->m_fHeight = nHeight;
	return layer;
}

COLORREF StaticLayer::BackgroundColor() const { return m_cBackgroundColor; }
void StaticLayer::SetBackgroundColor(COLORREF color) {
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
	m_cBackgroundColor = color;
}

Rect StaticLayer::Frame() const {
	float fWidth = m_fWidth / m_fScaleX;
	float fHeight = m_fHeight / m_fScaleY;
	float fLeft = m_fPositionX - fWidth * m_fAnchorX;
	float fTop = m_fPositionY - fHeight * m_fAnchorY; 
	return Rect(fLeft, fTop, fWidth, fHeight);
}
void StaticLayer::SetFrame(Rect frame) {
	Rect bounds = Rect(m_fOffsetX, m_fOffsetY, frame.size.width * m_fScaleX, frame.size.height * m_fScaleY);
	SetBounds(bounds);

	Point position = Point{
		frame.origin.x + frame.size.width * m_fAnchorX,
		frame.origin.y + frame.size.height * m_fAnchorY
	};
	SetPosition(position);
}

Rect StaticLayer::Bounds() const {
	return Rect(m_fOffsetX, m_fOffsetY, m_fWidth, m_fHeight);
}
void StaticLayer::SetBounds(Rect bounds) {
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
	SetContentOffset(Point{ roundf(bounds.origin.x), roundf(bounds.origin.y) });
	m_fWidth = roundf(bounds.size.width);
	m_fHeight = roundf(bounds.size.height);
}

Point StaticLayer::AnchorPoint() const { return Point{ m_fAnchorX, m_fAnchorY }; }
void StaticLayer::SetAnchorPoint(Point anchor) {
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
	m_fAnchorX = anchor.x;
	m_fAnchorY = anchor.y;
}

Point StaticLayer::Position() const { return Point{ m_fPositionX, m_fPositionY }; }
void StaticLayer::SetPosition(Point position) {
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
	m_fPositionX = roundf(position.x);
	m_fPositionY = roundf(position.y);
}

Size StaticLayer::Scale() const { return Size{ m_fScaleX, m_fScaleY }; }
void StaticLayer::SetScale(Size scale) {
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
	m_fScaleX = scale.width;
	m_fScaleY = scale.height;
}

Point StaticLayer::ContentOffset() const { return Point{ m_fOffsetX, m_fOffsetY }; }
void StaticLayer::SetContentOffset(Point offset) {
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
	m_fOffsetX = offset.x;
	m_fOffsetY = offset.y;
}

Size StaticLayer::ContentSize() const { return Size{ m_fContentWidth, m_fContentHeight }; }
void StaticLayer::SetContentSize(Size size) {
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
	m_fContentWidth = size.width;
	m_fContentHeight = size.height;
}

float StaticLayer::Rotate() const { return m_fRotate; }
void StaticLayer::SetRotate(float rotate) {
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
	m_fRotate = rotate;
}

float StaticLayer::CornerRadius() const { return m_fCornerRadius; }
void StaticLayer::SetCornerRadius(float radius) {
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
	m_fCornerRadius = radius;
}

float StaticLayer::ShadowOpacity() const { return m_fShadowOpacity; }
void StaticLayer::SetShadowOpacity(float opacity) {
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
	m_fShadowOpacity = opacity;
}

float StaticLayer::ShadowRadius() const { return m_fShadowRadius; }
void StaticLayer::SetShadowRadius(float radius) {
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
	m_fShadowRadius = roundf(radius);
}

gdiplus::Image* StaticLayer::Content() const { return m_spContent; }
void StaticLayer::SetContent(Image* pImage) {
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
	m_spContent = pImage;
}

bool StaticLayer::Hidden() const {
	return m_bHidden;
}

void StaticLayer::SetHidden(bool hidden) {
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
	m_bHidden = hidden;
}

void StaticLayer::SetNeedsDisplay() {
	m_bNeedsDisplay = true;
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
}

void StaticLayer::AddLayer(StaticLayer* pLayer) {
	assert(pLayer != nullptr);
	if (m_spParentLayer) m_spParentLayer->SetNeedsDisplay();
	pLayer->RemoveFromSuperlayer();
	m_aSublayers.Add(pLayer);
	pLayer->m_spParentLayer = this;
}

void StaticLayer::RemoveFromSuperlayer() {
	if (m_spParentLayer) {
		for (size_t i = 0; i < m_spParentLayer->m_aSublayers.GetCount(); i++) {
			if (m_spParentLayer->m_aSublayers[i] == this) {
				m_spParentLayer->m_aSublayers.RemoveAt(i);
				break;
			}
		}
		m_spParentLayer = nullptr;
	}
}

void StaticLayer::DrawString(const std::wstring& text, float fontSize, Color color, TextAlignment textAlignment, TextVerticalAlignment textVerticalAlignment, Matrix* pMatrix) {
	ui::Size size = Bounds().size;
	if (m_spFramebuffer == nullptr || m_spFramebuffer->m_nWidth < size.width || m_spFramebuffer->m_nHeight < size.height) {
		m_spFramebuffer = new Framebuffer((uint16_t)size.width, (uint16_t)size.height);
	}

	NONCLIENTMETRICSW metrics = { sizeof(metrics) };
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);

	Gdiplus::Font font = Gdiplus::Font(metrics.lfMessageFont.lfFaceName, fontSize);
	m_spFramebuffer->m_spGraphics->Clear(BackgroundColor());
	m_spFramebuffer->m_spGraphics->DrawString(text, &font, color.argb, textAlignment, textVerticalAlignment, Gdiplus::RectF(0, 0, m_fWidth, m_fHeight), pMatrix);
}

void StaticLayer::Draw(Graphics* pGraphics, Matrix* pMatrix) const {
	if (m_spContent != nullptr && m_fContentWidth > 0 && m_fContentHeight > 0) {
		pGraphics->DrawImage(
			m_spContent,
			Gdiplus::RectF(0, 0, m_fContentWidth, m_fContentHeight),
			Gdiplus::RectF(0, 0, m_fWidth, m_fHeight),
			pMatrix
		);
	}
}

void StaticLayer::Paint(HDC hDC) const {
	RefPtr<Graphics> pGraphics = Graphics::FromHDC(hDC);
	RefPtr<Matrix> pMatrix = new Matrix;
	Paint(pGraphics, pMatrix);
}

void StaticLayer::Paint(Graphics* pGraphics, Matrix* pMatrix) const {
	assert(pGraphics != nullptr);
	assert(pMatrix != nullptr);

	if (Hidden()) {
		if (m_spFramebuffer) m_spFramebuffer->m_spGraphics->Clear(0);
		return;
	}

	RefPtr<Matrix> pCurrent = pMatrix->Clone();
	pCurrent->Translate(m_fPositionX, m_fPositionY);
	pCurrent->Rotate(m_fRotate);
	pCurrent->Scale(1.0f / m_fScaleX, 1.0f / m_fScaleY);
	pCurrent->Translate(-(m_fWidth * m_fAnchorX + m_fOffsetX), -(m_fHeight * m_fAnchorY + m_fOffsetY));

	if (isNeedsOffscreenRender()) {
		RefPtr<Matrix> pLocal = new Matrix;
		pLocal->Translate(-m_fOffsetX, -m_fOffsetY);

		if (m_spFramebuffer == nullptr || m_fWidth > m_spFramebuffer->m_nWidth || m_fHeight > m_spFramebuffer->m_nHeight) {
			m_spFramebuffer = new Framebuffer(uint16_t(m_fWidth), uint16_t(m_fHeight));
			m_bNeedsDisplay = true;
		}

		if (m_bNeedsDisplay) {

			if (m_fCornerRadius > 0) {
				m_spFramebuffer->m_spGraphics->SetClip(Region::CreateRoundRectRegion(Rect(0, 0, m_fWidth, m_fHeight), m_fCornerRadius));
			}

			m_spFramebuffer->m_spGraphics->Clear(m_cBackgroundColor);

			Draw(m_spFramebuffer->m_spGraphics, pLocal);

			for (size_t i = 0; i < m_aSublayers.GetCount(); i++) {
				RefPtr<StaticLayer> layer = m_aSublayers[i];
				layer->Paint(m_spFramebuffer->m_spGraphics, pLocal);
			}

			m_spFramebuffer->m_spGraphics->SetClip(nullptr);

			if (m_fShadowOpacity > 0.0f && m_fShadowRadius > 0.0f) {
				m_spShadowBuffer = Image::CreateShadow(m_spFramebuffer->m_spImage, (int)m_fShadowRadius, m_fShadowOpacity);
			} else {
				m_spShadowBuffer = nullptr;
			}

			m_bNeedsDisplay = false;
		}
		float fWidth = m_fWidth;
		float fHeight = m_fHeight;
		float fLeft = m_fPositionX - fWidth * m_fAnchorX;
		float fTop = m_fPositionY - fHeight * m_fAnchorY;

		if (m_spShadowBuffer) {
			pGraphics->DrawImage(
				m_spShadowBuffer,
				Gdiplus::RectF(0, 0, m_spShadowBuffer->width(), m_spShadowBuffer->height()),
				Gdiplus::RectF(
					-m_fShadowRadius * 2 + m_fShadowOffsetX,
					-m_fShadowRadius * 2 + m_fShadowOffsetY,
					m_spShadowBuffer->width(),
					m_spShadowBuffer->height()
				),
				pCurrent
			);
		}
		pGraphics->DrawImage(
			m_spFramebuffer->m_spImage,
			Gdiplus::RectF(-m_fOffsetX, -m_fOffsetY, m_fWidth, m_fHeight),
			Gdiplus::RectF(0, 0, fWidth, fHeight),
			pCurrent
		);
	} else {
		if (m_cBackgroundColor != 0) pGraphics->DrawRect(m_cBackgroundColor, Gdiplus::RectF(0, 0, m_fWidth, m_fHeight), pCurrent);

		Draw(pGraphics, pCurrent);

		for (size_t i = 0; i < m_aSublayers.GetCount(); i++) {
			RefPtr<StaticLayer> layer = m_aSublayers[i];
			layer->Paint(pGraphics, pCurrent);
		}
	}
}

Color gdiplus::Layer::backgroundColor() const {
	return Color(_layer->BackgroundColor());
}

void gdiplus::Layer::setBackgroundColor(Color color) {
	_layer->SetBackgroundColor(color.argb);
}

Rect gdiplus::Layer::frame() const {
	return _layer->Frame();
}

void gdiplus::Layer::setFrame(Rect frame) {
	_layer->SetFrame(frame);
}

Rect gdiplus::Layer::bounds() const {
	return _layer->Bounds();
}

void gdiplus::Layer::setBounds(Rect bounds) {
	_layer->SetBounds(bounds);
}

Point gdiplus::Layer::anchorPoint() const {
	return _layer->AnchorPoint();
}

void gdiplus::Layer::setAnchorPoint(Point anchor) {
	_layer->SetAnchorPoint(anchor);
}

Point gdiplus::Layer::position() const {
	return _layer->Position();
}

void gdiplus::Layer::setPosition(Point position) {
	_layer->SetPosition(position);
}

Size gdiplus::Layer::scale() const {
	return _layer->Scale();
}

void gdiplus::Layer::setScale(Size scale) {
	_layer->SetScale(scale);
}

Point gdiplus::Layer::contentOffset() const {
	return _layer->ContentOffset();
}

void gdiplus::Layer::setContentOffset(Point offset) {
	_layer->SetContentOffset(offset);
}

Size gdiplus::Layer::contentSize() const {
	return _layer->ContentSize();
}

void gdiplus::Layer::setContentSize(Size size) {
	_layer->SetContentSize(size);
}

float gdiplus::Layer::rotate() const {
	return _layer->Rotate();
}

void gdiplus::Layer::setRotate(float rotate) {
	_layer->SetRotate(rotate);
}

float gdiplus::Layer::cornerRadius() const {
	return _layer->CornerRadius();
}

void gdiplus::Layer::setCornerRadius(float radius) {
	_layer->SetCornerRadius(radius);
}

float gdiplus::Layer::shadowOpacity() const {
	return _layer->ShadowOpacity();
}

void gdiplus::Layer::setShadowOpacity(float opacity) {
	_layer->SetShadowOpacity(opacity);
}

float gdiplus::Layer::shadowRadius() const {
	return _layer->ShadowRadius();
}

void gdiplus::Layer::setShadowRadius(float radius) {
	_layer->SetShadowRadius(radius);
}

void gdiplus::Layer::setNeedsDisplay() {
	_layer->SetNeedsDisplay();
}

Object* gdiplus::Layer::content() const {
	return _layer->Content();
}

void gdiplus::Layer::setContent(Object* image) {
	_layer->SetContent(RefPtr<Object>(image).as<gdiplus::Image>());
}

bool gdiplus::Layer::hidden() const {
	return _layer->Hidden();
}

void gdiplus::Layer::setHidden(bool hidden) {
	_layer->SetHidden(hidden);
}

void gdiplus::Layer::addLayer(ui::Layer* layer) {
	_layer->AddLayer(((Layer*)layer)->_layer);
}

void gdiplus::Layer::removeFromSuperlayer() {
	_layer->RemoveFromSuperlayer();
}

void gdiplus::Layer::drawText(const std::wstring& text, float fontSize, Color color, TextAlignment textAlignment, TextVerticalAlignment textVerticalAlignment) {
	RefPtr<Token> spToken = Token::Get();

	RefPtr<Matrix> matrix = new Matrix;

	_layer->DrawString(text, fontSize, color, textAlignment, textVerticalAlignment, matrix);
}

void gdiplus::Layer::paint(HDC hDC) {
	_layer->Paint(hDC);
}

void gdiplus::Layer::paint(RefPtr<render::RGBAVideoSource> source) const {
	RefPtr<Token> spToken = Token::Get();

	ui::Size size = bounds().size;
	if (size.width * size.height < 1) return;

	RefPtr<Matrix> matrix = new Matrix;

	RefPtr<Image> image = Image::Create((int32_t)size.width, (int32_t)size.height);
	_layer->Paint(Graphics::FromImage(image), matrix);
	source->update(image->Bitmap());
}

RefPtr<ui::Image> ui::Image::file(const std::wstring& path) {
	RefPtr<Token> spToken = Token::Get();
	return new gdiplus::Image(Gdiplus::Bitmap::FromFile(path.c_str()));
}

RefPtr<ui::Image> ui::Image::argb(uint32_t width, uint32_t height, const std::function<COLORREF(uint32_t x, uint32_t y)>& colors) {
	RefPtr<Token> spToken = Token::Get();
	Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
	if (bitmap) {
		Gdiplus::Rect rect(0, 0, width, height);
		Gdiplus::BitmapData bitmapData = { 0 };
		bitmap->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData);
		uint8_t* data = (uint8_t*)bitmapData.Scan0;
		for (uint32_t y = 0; y < bitmapData.Height; y++) {
			for (uint32_t x = 0; x < bitmapData.Width; x++) {
				(COLORREF&)data[y * bitmapData.Stride + x * 4] = colors(x, y);
			}
		}
		bitmap->UnlockBits(&bitmapData);
	}
	return new gdiplus::Image(bitmap);
}
