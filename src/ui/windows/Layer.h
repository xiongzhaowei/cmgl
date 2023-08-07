#pragma once

OMP_UI_WINDOWS_NAMESPACE_BEGIN

namespace gdiplus {
    class Token;
	class ImageData;
    class Image;
    class Matrix;
    class Path;
    class Region;
    class Graphics;
    class Framebuffer;
    class StaticLayer;
    class Layer;
}

class gdiplus::Token : public Object {
	ULONG_PTR m_hToken = 0;
	Gdiplus::GdiplusStartupInput m_input = { 0 };
public:
	Token();
	~Token();

	static WeakPtr<Token> shared;
	static RefPtr<Token> Get();
};

class gdiplus::Matrix : public Object {
	RefPtr<Token> m_spToken = Token::Get();
	Gdiplus::Matrix* m_pMatrix;
public:
	Matrix() : m_pMatrix(new Gdiplus::Matrix) {}

	Matrix(Gdiplus::Matrix* pMatrix) : m_pMatrix(pMatrix) { assert(pMatrix != nullptr); }

	Matrix* Clone() const { return new Matrix(m_pMatrix->Clone()); }

	void Multiply(const Matrix* matrix) {
		m_pMatrix->Multiply(matrix->m_pMatrix);
	}

	void Translate(float offsetX, float offsetY) {
		m_pMatrix->Translate(offsetX, offsetY);
	}

	void Scale(float scaleX, float scaleY) {
		m_pMatrix->Scale(scaleX, scaleY);
	}

	void Rotate(float angle) {
		m_pMatrix->Rotate(angle);
	}

	friend class Graphics;
};

class gdiplus::ImageData : public ui::ImageData {
	RefPtr<Token> m_spToken = Token::Get();
	Gdiplus::Bitmap* m_pImage;
public:
	ImageData(Gdiplus::Bitmap* pImage);
	~ImageData();

	float width() const override { return (float)m_pImage->GetWidth(); }
	float height() const override { return (float)m_pImage->GetHeight(); }
	void load(const std::function<void(int32_t width, int32_t height, int32_t stride, void* bytes)>& callback) override;
};

class gdiplus::Image : public ui::Image {
	RefPtr<Token> m_spToken = Token::Get();
	Gdiplus::Bitmap* m_pImage;
public:
	Image(Gdiplus::Bitmap* pImage);
	~Image();

	Gdiplus::Bitmap* Bitmap() const { return m_pImage; }
	float width() const override { return (float)m_pImage->GetWidth(); }
	float height() const override { return (float)m_pImage->GetHeight(); }

	static RefPtr<Image> FromFile(LPCWSTR lpszPath);
	static RefPtr<Image> FromHICON(HICON hIcon);
	static RefPtr<Image> Create(int width, int height);
	static RefPtr<Image> CreateShadow(RefPtr<Image> image, int radius, float opacity);

	friend class Graphics;
};

class gdiplus::Path : public Object {
	RefPtr<Token> m_spToken = Token::Get();
	Gdiplus::GraphicsPath* m_pPath;
public:
	Path(Gdiplus::GraphicsPath* pPath);
	~Path();
};

class gdiplus::Region : public Object {
	RefPtr<Token> m_spToken = Token::Get();
	Gdiplus::Region* m_pRegion;
public:
	Region(Gdiplus::Region* pRegion);
	~Region();

	static RefPtr<Region> CreateRoundRectRegion(Rect rect, float radius);

	friend class Graphics;
};

class gdiplus::Graphics : public Object {
	RefPtr<Token> m_spToken = Token::Get();
	RefPtr<Region> m_spRegion;
	Gdiplus::Graphics* m_pGraphics;
public:
	Graphics(Gdiplus::Graphics* pGraphics);
	~Graphics();

	bool DrawImage(Image* pImage, Gdiplus::RectF source, Gdiplus::RectF target, Matrix* transform);
	bool DrawString(const std::wstring& text, Gdiplus::Font* font, COLORREF color, TextAlignment textAlignment, TextVerticalAlignment textVerticalAlignment, Gdiplus::RectF target, Matrix* transform);
	bool DrawRect(COLORREF color, Gdiplus::RectF target, Matrix* transform);
	bool Clear(COLORREF color);
	void SetClip(Region* region);

	static RefPtr<Graphics> FromImage(Image* image);
	static RefPtr<Graphics> FromWindow(HWND hWnd);
	static RefPtr<Graphics> FromHDC(HDC hDC);
};

class gdiplus::Framebuffer : public Object {
	RefPtr<Token> m_spToken = Token::Get();
	RefPtr<Image> m_spImage;
	RefPtr<Graphics> m_spGraphics;
	const uint16_t m_nWidth;
	const uint16_t m_nHeight;
public:
	Framebuffer(uint16_t nWidth, uint16_t nHeight);

	friend class StaticLayer;
};

class gdiplus::StaticLayer : public Object {
	WeakPtr<StaticLayer> m_spParentLayer;
	RefPtr<Image> m_spContent;
	ATL::CAtlArray<RefPtr<StaticLayer>> m_aSublayers;

	mutable bool m_bNeedsDisplay = false;
	mutable RefPtr<Framebuffer> m_spFramebuffer;
	mutable RefPtr<Image> m_spShadowBuffer;

	COLORREF m_cBackgroundColor = 0;
	float m_fOffsetX = 0.0f;
	float m_fOffsetY = 0.0f;
	float m_fContentWidth = 0.0f;
	float m_fContentHeight = 0.0f;
	float m_fWidth = 0.0f;
	float m_fHeight = 0.0f;
	float m_fAnchorX = 0.5f;
	float m_fAnchorY = 0.5f;
	float m_fPositionX = 0.0f;
	float m_fPositionY = 0.0f;
	float m_fScaleX = 1.0f;
	float m_fScaleY = 1.0f;
	float m_fRotate = 0.0f;
	float m_fAlpha = 1.0f;
	float m_fCornerRadius = 0.0f;
	float m_fShadowOpacity = 0.0f;
	float m_fShadowRadius = 0.0f;
	float m_fShadowOffsetX = 0.0f;
	float m_fShadowOffsetY = 0.0f;
	bool m_bHidden = false;
public:
	static RefPtr<StaticLayer> Create(uint16_t nWidth, uint16_t nHeight);

	COLORREF BackgroundColor() const;
	void SetBackgroundColor(COLORREF color);

	Rect Frame() const;
	void SetFrame(Rect frame);

	Rect Bounds() const;
	void SetBounds(Rect bounds);

	Point AnchorPoint() const;
	void SetAnchorPoint(Point anchor);

	Point Position() const;
	void SetPosition(Point position);

	Size Scale() const;
	void SetScale(Size scale);

	Point ContentOffset() const;
	void SetContentOffset(Point offset);

	Size ContentSize() const;
	void SetContentSize(Size size);

	float Rotate() const;
	void SetRotate(float rotate);

	float CornerRadius() const;
	void SetCornerRadius(float radius);

	float ShadowOpacity() const;
	void SetShadowOpacity(float opacity);

	float ShadowRadius() const;
	void SetShadowRadius(float radius);

	Image* Content() const;
	void SetContent(Image* pImage);

	bool Hidden() const;
	void SetHidden(bool hidden);

	void SetNeedsDisplay();

	void AddLayer(StaticLayer* pLayer);
	void RemoveFromSuperlayer();

	void DrawString(const std::wstring& text, float fontSize, Color color, TextAlignment textAlignment, TextVerticalAlignment textVerticalAlignment, Matrix* pMatrix);

	bool isNeedsOffscreenRender() const { return true; }

	void Draw(Graphics* pGraphics, Matrix* pMatrix) const;
	void Paint(HDC hDC) const;
	void Paint(Graphics* pGraphics, Matrix* pMatrix) const;
};

class gdiplus::Layer : public ui::Layer {
	RefPtr<StaticLayer> _layer = new StaticLayer;
public:
	Color backgroundColor() const override;
	void setBackgroundColor(Color color) override;

	Rect frame() const override;
	void setFrame(Rect frame) override;

	Rect bounds() const override;
	void setBounds(Rect bounds) override;

	Point anchorPoint() const override;
	void setAnchorPoint(Point anchor) override;

	Point position() const override;
	void setPosition(Point position) override;

	Size scale() const override;
	void setScale(Size scale) override;

	Point contentOffset() const override;
	void setContentOffset(Point offset) override;

	Size contentSize() const override;
	void setContentSize(Size size) override;

	float rotate() const override;
	void setRotate(float rotate) override;

	float cornerRadius() const override;
	void setCornerRadius(float radius) override;

	float shadowOpacity() const override;
	void setShadowOpacity(float opacity) override;

	float shadowRadius() const override;
	void setShadowRadius(float radius) override;

	Object* content() const override;
	void setContent(Object* image) override;

	bool hidden() const;
	void setHidden(bool hidden);

	void setNeedsDisplay() override;

	void addLayer(ui::Layer* layer) override;
	void removeFromSuperlayer() override;

	void drawText(const std::wstring& text, float fontSize, Color color, TextAlignment textAlignment, TextVerticalAlignment textVerticalAlignment) override;

	void paint(HDC hDC);
	void paint(RefPtr<render::RGBAVideoSource> source) const;
};

OMP_UI_WINDOWS_NAMESPACE_END
