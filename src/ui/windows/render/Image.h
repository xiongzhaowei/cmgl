
OMP_UI_WINDOWS_NAMESPACE_BEGIN

class ImageDataSource : public render::DataSource {
    glm::vec2 _size;
    RefPtr<render::Texture> _texture;
    std::function<RefPtr<ImageData>()> _loader;
public:
    ImageDataSource(const std::function<RefPtr<ImageData>()>& loader);

    void load(RefPtr<render::RenderContext> context) override;
    void unload(RefPtr<render::RenderContext> context) override;
    glm::vec2 size() const override;
    void draw(
        RefPtr<render::RenderContext> context,
        RefPtr<render::Framebuffer> framebuffer,
        const glm::mat4& globalMatrix,
        const glm::mat4& localMatrix,
        const glm::mat4& clipMatrix,
        const glm::vec2& size,
        float alpha
    );
};

class Image : public ui::Image {
    glm::vec2 _size;
    std::function<RefPtr<ImageData>()> _loader;
    WeakPtr<ImageDataSource> _renderObject;
public:
    Image(const std::function<RefPtr<ImageData>()>& loader);

    float width() const;
    float height() const;
    RefPtr<render::DataSource> renderObject();
};

OMP_UI_WINDOWS_NAMESPACE_END
