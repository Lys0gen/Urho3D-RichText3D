#ifndef __RICH_WIDGET_TEXT_H__
#define __RICH_WIDGET_TEXT_H__
#pragma once

#include "engine/richtext/rich_batch.h"
#include "Urho3D/Graphics/Material.h"

namespace Urho3D
{
class Font;
class FontFace;
class RichFontProvider;

/// A mesh that displays text quads with a single font/size.
class RichWidgetText: public RichWidgetBatch
{
    URHO3D_OBJECT(RichWidgetText, RichWidgetBatch)
public:
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Contructor.
    RichWidgetText(Context* context);
    /// Destructor.
    virtual ~RichWidgetText();

    /// Draw a quad.
    void DrawQuad(const Rect& vertices, float z, const Rect& texCoords, const Color& color);
    /// Draw a glyph.
    void DrawGlyph(const Rect& texCoords, float x, float y, float z, float width, float height, const Color& color);
    /// Draw a glyph, scaled depending on the bitmap font and pointsize.
    void DrawGlyphScaled(const Rect& texCoords, float x, float y, float z, float width, float height, const Vector2& scale, const Color& color);
    /// Add text.
    void AddText(const String& text, const Vector3& pos, const Color& color);
    /// Set the font.
    void SetFont(const String& fontname, int pointsize, bool bold = false, bool italic = false);
    /// Get the font face (only valid after SetFont).
    FontFace* GetFontFace() const { return font_face_; }
    /// Calculate text extents with the current font
    Vector2 CalculateTextExtents(const String& text);
    /// Row height
    float GetRowHeight() const;
    /// RichFontProvider calls this when it resolves the font
    void SetFontResource(Font* font);
    // override IsEmpty() to return false while requested a font
    bool IsEmpty() const override;
private:
    Font * font_{};
    FontFace* font_face_{};
    int pointsize_{};
    bool bold_{};
    bool italic_{};
    Vector2 bitmap_font_rescale_{Vector2::ONE};
    bool pending_font_request_{false};
};

} // namespace Urho3D

#endif
