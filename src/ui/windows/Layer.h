#pragma once

OMP_UI_WINDOWS_NAMESPACE_BEGIN

namespace gdiplus {
    class Token;
    class Image;
    class Matrix;
    class Path;
    class Region;
    class Graphics;
    class Framebuffer;
    class StaticLayer;
    class Layer;
}

class gdiplus::Token : public SupportWeak<Token> {
	ULONG_PTR m_hToken = 0;
	Gdiplus::GdiplusStartupInput m_input = { 0 };
public:
	Token();
	~Token();

	static WeakPtr<Token> shared;
	static CComPtr<Token> Get();
};

class gdiplus::Matrix : public RefCounted<Matrix> {
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

class gdiplus::Image : public RefCounted<Image> {
	CComPtr<Token> m_spToken = Token::Get();
	Gdiplus::Bitmap* m_pImage;
public:
	Image(Gdiplus::Bitmap* pImage);
	~Image();

	float Width() const { return (float)m_pImage->GetWidth(); }
	float Height() const { return (float)m_pImage->GetHeight(); }

	static CComPtr<Image> FromFile(LPCWSTR lpszPath);
	static CComPtr<Image> FromHICON(HICON hIcon);
	static CComPtr<Image> CreateShadow(CComPtr<Image> image, int radius, float opacity);

	friend class Graphics;
};

class gdiplus::Path : public RefCounted<Path> {
	CComPtr<Token> m_spToken = Token::Get();
	Gdiplus::GraphicsPath* m_pPath;
public:
	Path(Gdiplus::GraphicsPath* pPath);
	~Path();
};

class gdiplus::Region : public RefCounted<Region> {
	CComPtr<Token> m_spToken = Token::Get();
	Gdiplus::Region* m_pRegion;
public:
	Region(Gdiplus::Region* pRegion);
	~Region();

	static CComPtr<Region> CreateRoundRectRegion(Rect rect, float radius);

	friend class Graphics;
};

class gdiplus::Graphics : public RefCounted<Graphics> {
	CComPtr<Token> m_spToken = Token::Get();
	CComPtr<Region> m_spRegion;
	Gdiplus::Graphics* m_pGraphics;
public:
	Graphics(Gdiplus::Graphics* pGraphics);
	~Graphics();

	bool DrawImage(Image* pImage, Gdiplus::RectF source, Gdiplus::RectF target, Matrix* transform);
	bool DrawRect(COLORREF color, Gdiplus::RectF target, Matrix* transform);
	bool Clear(COLORREF color);
	void SetClip(Region* region);

	static CComPtr<Graphics> FromWindow(HWND hWnd);
	static CComPtr<Graphics> FromHDC(HDC hDC);
};

class gdiplus::Framebuffer : public RefCounted<Framebuffer> {
	CComPtr<Token> m_spToken = Token::Get();
	CComPtr<Image> m_spImage;
	CComPtr<Graphics> m_spGraphics;
	const uint16_t m_nWidth;
	const uint16_t m_nHeight;
public:
	Framebuffer(uint16_t nWidth, uint16_t nHeight);

	friend class StaticLayer;
};

class gdiplus::StaticLayer : public SupportWeak<StaticLayer> {
	WeakPtr<StaticLayer> m_spParentLayer;
	CComPtr<Image> m_spContent;
	ATL::CAtlArray<CComPtr<StaticLayer>> m_aSublayers;

	mutable bool m_bNeedsDisplay = false;
	mutable CComPtr<Framebuffer> m_spFramebuffer;
	mutable CComPtr<Image> m_spShadowBuffer;

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

public:
	static CComPtr<StaticLayer> Create(uint16_t nWidth, uint16_t nHeight);

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

	void SetNeedsDisplay();
	void SetContent(Image* pImage);

	void AddLayer(StaticLayer* pLayer);
	void RemoveFromSuperlayer();

	bool isNeedsOffscreenRender() const { return true; }

	void Draw(Graphics* pGraphics, Matrix* pMatrix) const;
	void Paint(HDC hDC) const;
	void Paint(Graphics* pGraphics, Matrix* pMatrix) const;
};

class gdiplus::Layer : public ui::Layer {
	CComPtr<StaticLayer> _layer = new StaticLayer;
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

	void setNeedsDisplay() override;
	void setContent(void* image) override;

	void addLayer(ui::Layer* layer) override;
	void removeFromSuperlayer() override;

	void paint(HDC hDC);
};

OMP_UI_WINDOWS_NAMESPACE_END
