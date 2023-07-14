
struct tagMSG;

OMP_UI_NAMESPACE_BEGIN

struct Point {
    float x;
    float y;

    Point();
    Point(float x, float y);
};

struct Size {
    float width;
    float height;

    Size();
    Size(float width, float height);
};

struct Rect {
    Point origin;
    Size size;

    Rect();
    Rect(float x, float y, float width, float height);
};

union Color {
    struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    };
    uint32_t argb;

    Color(uint32_t color);
};

enum class TextAlignment {
    none,
    left,
    center,
    right,
};

enum class TextVerticalAlignment {
    none,
    top,
    center,
    bottom,
};

class Image : public Object {
public:
	virtual float Width() const = 0;
	virtual float Height() const = 0;

	static RefPtr<Image> file(const std::wstring& path);
	static RefPtr<ui::Image> argb(uint32_t width, uint32_t height, const std::function<COLORREF(uint32_t x, uint32_t y)>& colors);
};

typedef struct tagMSG NativeEvent;
class View;

struct MouseEvent {
    const NativeEvent& native;
    Point pt;

    MouseEvent(const NativeEvent& native, float x, float y);
    Point localPoint(RefPtr<View> view) const;
};

struct KeyboardEvent {
    const NativeEvent& native;

    KeyboardEvent(const NativeEvent& native);
};

OMP_UI_NAMESPACE_END
