#ifndef __RICH_WIDGET_H__
#define __RICH_WIDGET_H__
#pragma once

#include "rich_batch.h"
#include "Urho3D/Container/Ptr.h"
#include "Urho3D/Math/Rect.h"
#include "Urho3D/Graphics/VertexBuffer.h"
#include "Urho3D/UI/Text.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Core/Context.h"
#include "Urho3D/Graphics/Geometry.h"
#include "Urho3D/Graphics/Material.h"
//#include "Urho3D/Urho3DAll.h"

namespace Urho3D
{

/// Ticker type
enum TickerType
{
    TickerType_None,
    TickerType_Horizontal,
    TickerType_Vertical,
};

/// Ticker direction
enum TickerDirection
{
    TickerDirection_Negative,
    TickerDirection_Positive,
};

/// Text wrapping
enum TextWrapping
{
    WRAP_NONE,
    WRAP_WORD
};

/// Font description of a text block
struct FontState
{
    String face;
    unsigned size{};
    bool bold{};
    bool italic{};
};

struct BlockFormat
{
  /// Font description
  FontState font;
  /// Alignment
  HorizontalAlignment align{HA_LEFT};

  Color color{Color::WHITE};
  bool underlined{};
  bool striked{};
  bool superscript{};
  bool subscript{};
};

/// A block of text or an image
struct TextBlock
{
    enum BlockType {
        BlockType_Text,
        BlockType_Image,
        BlockType_Plugin,
    };
    /// Type of block
    BlockType type{BlockType_Text};
    /// Text or image/material source, or tag data of plugins
    String text;

    BlockFormat format;

    float image_width{};
    float image_height{};
    bool is_visible{true};
    bool is_line_break{};
};

/// A line inside the text layout
struct TextLine
{
    int width{};
    int height{};

    int offset_x{};
    int offset_y{};

    HorizontalAlignment	align{HA_LEFT};
    Vector<TextBlock> blocks;
};

class RichWidgetBatch;
class RichWidget;

enum WidgetFlags
{
    WidgetFlags_GeometryDirty = 1,	// the widget needs redrawing of the quads
    WidgetFlags_ContentChanged = 2,	// an asset has changed, the widget needs to update its content
    WidgetFlags_All = 0xFFFFFFFF		// combination of all flags
};

/// Container for RichBatch items, each batch is a different text style or image element.
class RichWidget: public Drawable
{
    URHO3D_OBJECT(RichWidget, Drawable)
public:
    static const float unitsPerPixel;
    /// Register object factory. Drawable must be registered first.
    static void RegisterObject(Context* context);

    /// Constructor.
    RichWidget(Context* context);
    /// Destructor.
    virtual ~RichWidget();
    /// Set the size of the widget (all render items will be clipped to this size).
    void SetClipRegion(const IntRect& rect);
    /// Get the size of the widget, default IntRect::ZERO - no clipping.
    IntRect GetClipRegion() const { return clip_region_; }
    /// Set clip region to content size automatically, default - true
    void SetClipToContent(bool value);
    /// Get clip region to content size automatically
    bool GetClipToContent() const { return clip_to_content_; }
    /// Set the internal scale of the widget, e.g. object units per pixel.
    void SetInternalScale(const Vector2& scale);
    /// Get internal scale, default 1.0.
    Vector2 GetInternalScale() const { return internal_scale_; }
    /// Set the draw origin, e.g. the point in 3D local space where the render items draw.
    void SetDrawOrigin(const Urho3D::Vector3& point);
    /// Get the draw origin, e.g. the point in 3D local space where the render items draw, default Vector3::ZERO.
    Vector3 GetDrawOrigin() const {	return draw_origin_; }
    /// Set padding.
    void SetPadding(const IntRect& padding);
    /// Get padding, default IntRect::ZERO.
    IntRect GetPadding() const { return padding_; }
    /// Clear all render items.
    virtual void Clear();
    /// Templated add WidgetBatch.
    template<typename T> T* AddWidgetBatch();
    /// Templated cache a widget.
    template<typename T> T* CacheWidgetBatch(StringHash id);
    /// Creates a WidgetBatch with the specified texture and type. If it already exists, returns the existing instance.
    RichWidgetBatch* CacheWidgetBatchT(StringHash type, StringHash id);
    /// Remove all batches.
    void RemoveWidgetBatches();
    /// Remove unused batches (cached but not referenced).
    void RemoveUnusedWidgetBatches();
    /// Get a list of cached batches.
    const Vector<SharedPtr<RichWidgetBatch>>& GetWidgetBatches() const { return items_; }

    /// Draw all render items (UI).
    virtual void Draw(UIElement* uiElement, PODVector<UIBatch>& batches, PODVector<float>& vertexData, const IntRect& currentScissor);

    /// Set the flags of the widget. Uses WidgetFlags_XXX combination.
    void SetFlags(unsigned flags);
    /// Get the widget flags.
    unsigned GetFlags() const { return flags_; }
    /// Clear specified flags.
    void ClearFlags(unsigned flags_to_clear) { flags_ &= ~flags_to_clear; }
    /// Check if flag is up.
    bool IsFlagged(unsigned flags) const { return !!(flags_ & flags); }
    /// Set visible.
    void SetVisible(bool visible);
    /// Get visible.
    bool GetVisible() const {	return visible_; }
    /// Set z-bias order of the whole widget.
    void SetZBias(int zbias);
    /// Get z-bias order.
    int GetZBias() const { return zbias_;}
    /// Set alpha.
    void SetAlpha(float color);
    /// Get alpha.
    float GetAlpha() const { return alpha_; }
    /// Enable shadow effect.
    void SetShadowEnabled(bool shadow_enabled);
    /// Is shadow effect enabled ?
    bool GetShadowEnabled() const {	return shadow_enabled_; }
    /// Set shadow offset.
    void SetShadowOffset(const Vector4& shadow_offset);
    /// Get shadow offset.
    Vector4 GetShadowOffset() const { return shadow_offset_; }
    /// Set shadow color.
    void SetShadowColor(const Color& color);
    /// Get shadow color.
    Color GetShadowColor() const { return shadow_color_; }
    /// Set widget horizontal alignment.
    void SetHorizontalAlignment(HorizontalAlignment align);
    /// Get widget horizontal alignment.
    HorizontalAlignment GetHorizontalAlignment() const { return align_h_; }
    /// Set widget vertical alignment.
    void SetVerticalAlignment(VerticalAlignment align);
    /// Get widget vertical alignment.
    VerticalAlignment GetVerticalAlignment() const { return align_v_; }
    /// Get content size.
    Vector2 GetContentSize() const { return content_size_; }
    /// Set content size.
    void SetContentSize(Vector2 newSize) { content_size_ = newSize; }
    /// Set whether text has fixed size on screen (pixel-perfect) regardless of distance to camera. Works best when combined with face camera rotation. Default false.
    void SetFixedScreenSize(bool enable);
    /// Return whether text has fixed screen size.
    bool IsFixedScreenSize() const { return fixedScreenSize_; }
    /// Set how the text should rotate in relation to the camera. Default is to not rotate (FC_NONE.)
    void SetFaceCameraMode(FaceCameraMode mode);
    /// Return how the text rotates in relation to the camera.
    FaceCameraMode GetFaceCameraMode() const { return faceCameraMode_; }
    /// A cache of the used render items, all unused render items (those with no quads) will be freed.
    Vector<SharedPtr<RichWidgetBatch>> items_;
protected:
    friend class RichWidgetBatch;
    /// The clipping region, default 0, no clipping.
    IntRect clip_region_;
    /// Draw padding, default 0, no padding.
    IntRect padding_;
    /// Draw origin items inside are drawn from, default 0.
    Vector3 draw_origin_;
    /// Content scaling.
    Vector2 internal_scale_;
    /// Content size. Updated from subclasses.
    Vector2 content_size_;
    /// WidgetFlags_XXX combination.
    unsigned flags_;
    /// Specifies if anything would be drawn inside.
    bool visible_;
    /// Zbias for the whole widget, all batch materials will inherit this zbias.
    int zbias_;
    /// Alpha.
    float alpha_;
    /// Is shadow enabled ?
    bool shadow_enabled_;
    /// Shadow offset.
    Vector4 shadow_offset_;
    /// Shadow color.
    Color shadow_color_;
    /// Default material used.
    SharedPtr<Material> material_;
    /// UI batches generated from the widgets.
    PODVector<UIBatch> ui_batches_;
    /// Vertex data generated from the widgets.
    PODVector<float> ui_vertex_data_;
    /// Geometries.
    Vector<SharedPtr<Geometry>> geometries_;
    /// Vertex buffer.
    SharedPtr<VertexBuffer> vertex_buffer_;
    /// Link between item index and sourcebatch index
    PODVector<int> batch_index_to_item_index_;
    /// Horizontal alignment.
    HorizontalAlignment align_h_;
    /// Vertical alignment.
    VerticalAlignment align_v_;
    /// Clip content to its size (default true)
    bool clip_to_content_;

    /// Custom world transform for facing the camera automatically.
    Matrix3x4 customWorldTransform_;
    /// Text rotation mode in relation to the camera.
    FaceCameraMode faceCameraMode_;
    /// Minimal angle between text normal and look-at direction.
    float minAngle_;
    /// Fixed screen size flag.
    bool fixedScreenSize_;

    /// The clip region after scaling. TODO: remove
    Rect GetActualDrawArea(bool withPadding = true) const;
    /// Draw all render items.
    virtual void Draw();
    /// Update the geometries_
    void UpdateTextBatches(UIElement* uiElement = NULL, PODVector<UIBatch>* batches = NULL, PODVector<float>* vertexData = NULL, const IntRect* currentScissor = NULL);
    /// Update the geometry_ materials and SourceBatch from the UIBatch list.
    void UpdateTextMaterials();
    //void UpdateTextMaterials(UIElement* uiElement = NULL, PODVector<UIBatch>* batches = NULL, PODVector<float>* vertexData = NULL, const IntRect* currentScissor = NULL);
    /// Recalculate camera facing and fixed screen size.
    void CalculateFixedScreenSize(const FrameInfo& frame);

    /// Calculate distance and prepare batches for rendering. May be called from worker thread(s), possibly re-entrantly.
    void UpdateBatches(const FrameInfo& frame) override;

    /// Prepare geometry for rendering. Called from a worker thread if possible (no GPU update).
     void UpdateGeometry(const FrameInfo& frame) override;
    /// Return whether a geometry update is necessary, and if it can happen in a worker thread.
    UpdateGeometryType GetUpdateGeometryType() override;
    /// Recalculate the world-space bounding box.
    void OnWorldBoundingBoxUpdate() override;
};

template<typename T> T* RichWidget::AddWidgetBatch() {
    T* new_t = new T(context_);
    new_t->use_count_ = 1;
    items_.Push(new_t);
    new_t->SetNode(node_);
    return new_t;
}

template<typename T> T* RichWidget::CacheWidgetBatch(StringHash name) {
    return static_cast<T*>(CacheWidgetBatchT(T::GetTypeStatic(), name));
}

} // namespace Urho3D

#endif
