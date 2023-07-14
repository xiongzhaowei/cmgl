
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

    Color(uint32_t color) : argb(color) {}
};

typedef struct tagMSG NativeEvent;

struct MouseEvent {
    const NativeEvent& native;
    Point pt;

    MouseEvent(const NativeEvent& native, float x, float y);
};

struct KeyboardEvent {
    const NativeEvent& native;

    KeyboardEvent(const NativeEvent& native);
};

OMP_UI_NAMESPACE_END
