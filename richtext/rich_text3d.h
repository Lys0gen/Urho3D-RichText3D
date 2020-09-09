#ifndef __RICH_TEXT_3D_H__
#define __RICH_TEXT_3D_H__
#pragma once

#include "engine/richtext/rich_widget.h"

namespace Urho3D {

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

/// RichTextScrolledOut
URHO3D_EVENT(E_RICHTEXT_SCROLLED_OUT, RichTextScrolledOut) {
  URHO3D_PARAM(P_TEXT, Text);      // RichText component
}

/// RichText3D
class RichText3D: public RichWidget
{
    URHO3D_OBJECT(RichText3D, RichWidget)
public:
    /// Register object factory. Drawable must be registered first.
    static void RegisterObject(Context* context);

    /// Contructor.
    RichText3D(Context* context);
    /// Destructor.
    virtual ~RichText3D();

    /// Set display text inside the view.
    void SetText(const String& text);
    /// Get currently displayed text (as markup).
    const String& GetText() const;
    /// Set default font for blocks without formatting.
    void SetDefaultFont(const String& face, unsigned size);
    /// Get default font name
    String GetDefaultFontName() const { return default_format_.font.face; }
    /// Get default font size
    unsigned GetDefaultFontSize() const { return default_format_.font.size; }
    /// Set text color.
    void SetTextColor(const Color& color);
    /// Get text color.
    Color GetTextColor() const { return default_format_.color; }
    /// Set text alignment.
    void SetAlignment(HorizontalAlignment align);
    /// Get text alignment.
    HorizontalAlignment GetAlignment() const { return default_format_.align; }
    /// Set additional line spacing (can be negative).
    void SetLineSpacing(int line_spacing);
    /// Get additional line spacing.
    int GetLineSpacing() const { return line_spacing_; }
    /// Set word wrapping.
    void SetWrapping(bool wrapping);
    /// Get wrapping.
    bool GetWrapping() const { return wrapping_ == WRAP_WORD; }
    /// Set ticker type.
    void SetTickerType(TickerType type);
    /// Get ticker type.
    TickerType GetTickerType() const;
    /// Set ticker scroll direction.
    void SetTickerDirection(TickerDirection direction);
    /// Get ticker scroll direction.
    TickerDirection GetTickerDirection() const;
    /// Set ticker scroll speed.
    void SetTickerSpeed(float pixelspersecond);
    /// Get ticker scroll speed.
    float GetTickerSpeed() const;
    /// Set single line.
    void SetSingleLine(bool single_line);
    /// Get single line.
    bool GetSingleLine() const { return single_line_; }
    /// Reset the ticker to the beginning.
    void ResetTicker();
    /// Set ticker position (0-1 range).
    void SetTickerPosition(float tickerPosition);
    /// Get ticker position (0-1 range).
    float GetTickerPosition() const;
    /// Get font ResourceRef.
    ResourceRef GetFontAttr() const;
    /// Set font ResourceRef.
    void SetFontAttr(const ResourceRef& value);
    /// Get font size.
    int GetFontSizeAttr() const;
    /// Set font size.
    void SetFontSizeAttr(int size);
protected:
    /// The displayed text.
    String text_;
    /// Additional line spacing (can be negative).
    int line_spacing_;
    /// Ticker type.
    TickerType ticker_type_;
    /// Ticker direction.
    TickerDirection ticker_direction_;
    /// Ticker speed.
    float ticker_speed_;
    /// Default font state for unformatted text.
    BlockFormat default_format_;
    /// The lines of text.
    Vector<TextLine> lines_; // TODO: could be removed in the future.
    /// The scroll origin of the text (in ticker mode).
    Vector3 scroll_origin_;
    /// Is the text single line.
    bool single_line_;
    /// Ticker position (0-1).
    float ticker_position_;
    /// Wrapping
    TextWrapping wrapping_;

    /// Compile the text to render items.
    void CompileTextLayout();
    /// Arrange text blocks into the textview layout as lines.
    void ArrangeTextBlocks(Vector<TextBlock>& markup_blocks);
    /// Draw text lines to the widget.
    void DrawTextLines();

    /// Per-frame text animation.
    void UpdateTickerAnimation(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
};

} // namespace Urho3D

#endif
