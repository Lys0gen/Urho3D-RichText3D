#include "../Precompiled.h"

#include "../Core/Context.h"
#include "../Core/Profiler.h"
#include "../Graphics/Texture2D.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"
#include "../UI/Font.h"
#include "../UI/FontFace.h"
#include "../UI/Text.h"
#include "../Resource/Localization.h"
#include "../Resource/ResourceEvents.h"

#include "../DebugNew.h"

#include "rich_text3d.h"
#include "rich_batch_text.h"
#include "rich_batch_image.h"
#include "Urho3D/Core/StringUtils.h"
#include "rich_html_parser.h"
#include "rich_textui.h"
#include <limits.h>


namespace Urho3D
{

namespace
{

inline unsigned find_first_of(const String& str, const String& delimiters, unsigned offset = 0) {
  if (offset >= str.Length())
    return -1;

  for (unsigned i = offset; i < str.Length(); i++) {
    for (String::ConstIterator it = delimiters.Begin(); it != delimiters.End(); ++it)
      if ((*it) == str.At(i))
        return i;
  }
  return -1;
}


// Characters that split words
static const char splitter_chars[] = " \t,.:;";

// Old code kept for 1:1 compatibility
template <typename T>
inline void splitwords(const String& str, T& tokens, const String& delimiters = " ", bool trimEmpty = false) {
  unsigned pos, lastPos = 0;
  while(true) {
    pos = find_first_of(str, delimiters, lastPos);
    if(pos == String::NPOS) {
      pos = str.Length();
      if(pos != lastPos || !trimEmpty)
        tokens.Push(typename T::ValueType(str.CString() + lastPos,
          pos - lastPos));

      break;
    } else {
      if(pos != lastPos) {
        tokens.Push(typename T::ValueType(str.CString() + lastPos, pos - lastPos));
      }
      if (!trimEmpty)
        tokens.Push(typename T::ValueType(str.CString() + pos, 1 ));
    }

    lastPos = pos + 1;
  }
}

} // namespace


extern const char* textEffects[];

extern const char* ticker_types[];
extern const char* ticker_directions[];

static const float MIN_ROW_SPACING = 0.5f;

extern const char* GEOMETRY_CATEGORY;
extern const char* horizontalAlignments[];
extern const char* UI_CATEGORY;

RichTextUI::RichTextUI(Context* context) :
    UIElement(context),
    rowSpacing_(1.0f),
    charLocationsDirty_(true),
    autoSize_(false),
    selectionStart_(0),
    selectionLength_(0),
    textEffect_(TE_NONE),
    shadowOffset_(IntVector2(1, 1)),
    strokeThickness_(1),
    roundStroke_(false),
    effectColor_(Color::BLACK),
    effectDepthBias_(0.0f),
    rowHeight_(0),
	//ticker_type_(TickerType_None),
	//ticker_direction_(TickerDirection_Negative),
	//ticker_speed_(60),
	scroll_origin_(0.0f, 0.0f),
	single_line_(false),
	wrapping_(WRAP_WORD),
	//ticker_position_(0.0f),
	line_spacing_(0)
{
    // By default Text does not derive opacity from parent elements
    if(widget_.Get() == nullptr){
        widget_ = new RichWidget(context);
    }

    useDerivedOpacity_ = false;
    default_format_.color = Color::WHITE;
	default_format_.align = HA_LEFT;
    SetFont("Fonts/Anonymous Pro.ttf", 32);

    //SubscribeToEvent(Urho3D::E_SCENEUPDATE, URHO3D_HANDLER(RichTextUI, UpdateTickerAnimation));
}

RichTextUI::~RichTextUI() = default;

void RichTextUI::RegisterObject(Context* context)
{
    context->RegisterFactory<RichTextUI>(UI_CATEGORY);

	//URHO3D_COPY_BASE_ATTRIBUTES(RichWidget);
	//URHO3D_COPY_BASE_ATTRIBUTES(Drawable);
	URHO3D_COPY_BASE_ATTRIBUTES(UIElement);
	URHO3D_UPDATE_ATTRIBUTE_DEFAULT_VALUE("Use Derived Opacity", false);
	URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Font", GetFontAttr, SetFontAttr, ResourceRef, ResourceRef(Font::GetTypeStatic()), AM_FILE);
	URHO3D_ACCESSOR_ATTRIBUTE("Font Size", GetFontSize, SetFontSize, int, 14, AM_DEFAULT);
	URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Text", GetTextAttr, SetTextAttr, String, String::EMPTY, AM_FILE);
	//URHO3D_ENUM_ATTRIBUTE("Text Alignment", textAlignment_, horizontalAlignments, HA_LEFT, AM_FILE);
	URHO3D_ENUM_ACCESSOR_ATTRIBUTE("Text Alignment", GetTextAlignment, SetTextAlignment, HorizontalAlignment,
									horizontalAlignments, HA_LEFT, AM_DEFAULT);
	URHO3D_ATTRIBUTE("Row Spacing", float, rowSpacing_, 1.0f, AM_FILE);
	//URHO3D_ATTRIBUTE("Word Wrap", bool, wordWrap_, false, AM_FILE);
	URHO3D_ACCESSOR_ATTRIBUTE("Word Wrap", GetWrapping, SetWrapping, bool, true, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Auto Size", GetAutoSize, SetAutoSize, bool, false, AM_DEFAULT);
	//URHO3D_ACCESSOR_ATTRIBUTE("Auto Localizable", GetAutoLocalizable, SetAutoLocalizable, bool, false, AM_FILE);
	//URHO3D_ENUM_ATTRIBUTE("Text Effect", textEffect_, textEffects, TE_NONE, AM_FILE);
	URHO3D_ATTRIBUTE("Shadow Offset", IntVector2, shadowOffset_, IntVector2(1, 1), AM_FILE);
	URHO3D_ATTRIBUTE("Stroke Thickness", int, strokeThickness_, 1, AM_FILE);
	URHO3D_ATTRIBUTE("Round Stroke", bool, roundStroke_, false, AM_FILE);
	//URHO3D_ACCESSOR_ATTRIBUTE("Effect Color", GetEffectColor, SetEffectColor, Color, Color::BLACK, AM_FILE);
	URHO3D_ACCESSOR_ATTRIBUTE("Single Line", GetSingleLine, SetSingleLine, bool, false, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Line Spacing", GetLineSpacing, SetLineSpacing, int, 0, AM_DEFAULT);
	URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Color", GetTextColor, SetTextColor, Color, Color::WHITE, AM_DEFAULT);

/*
	URHO3D_ENUM_ACCESSOR_ATTRIBUTE("Ticker Type", GetTickerType, SetTickerType, TickerType,
									ticker_types, TickerType_None, AM_DEFAULT);
	URHO3D_ENUM_ACCESSOR_ATTRIBUTE("Ticker Direction", GetTickerDirection, SetTickerDirection, TickerDirection,
									ticker_directions, TickerDirection_Negative, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Ticker Speed", GetTickerSpeed, SetTickerSpeed, float, 120, AM_DEFAULT);


  URHO3D_ACCESSOR_ATTRIBUTE("Can Be Occluded", IsOccludee, SetOccludee, bool, true, AM_DEFAULT);*/




    // Change the default value for UseDerivedOpacity
    context->GetAttribute<RichTextUI>("Use Derived Opacity")->defaultValue_ = false;
}

void RichTextUI::ApplyAttributes()
{
    UIElement::ApplyAttributes();

    // Localize now if attributes were loaded out-of-order
    /*if (autoLocalizable_ && stringId_.Length())
    {
        auto* l10n = GetSubsystem<Localization>();
        text_ = l10n->Get(stringId_);
    }*/

    DecodeToUnicode();

    default_format_.font.size = Max(default_format_.font.size, 1);
    strokeThickness_ = Abs(strokeThickness_);
//    ValidateSelection();
    UpdateText();
}


void RichTextUI::SetTextColor(const Color& color)
{
    default_format_.color = color;
    widget_->SetFlags(WidgetFlags_ContentChanged);
}

void RichTextUI::SetLineSpacing(int line_spacing) {
	line_spacing_ = line_spacing;
	widget_->SetFlags(WidgetFlags_ContentChanged);
    charLocationsDirty_ = true;
}

void RichTextUI::SetSingleLine(bool single_line)
{
    single_line_ = single_line;
    widget_->SetFlags(WidgetFlags_ContentChanged);
    charLocationsDirty_ = true;
}

void RichTextUI::OnResize(const IntVector2& newSize, const IntVector2& delta)
{
    if (wrapping_)
        UpdateText(true);
    else
        charLocationsDirty_ = true;
}

void RichTextUI::OnIndentSet()
{
    charLocationsDirty_ = true;
}

bool RichTextUI::SetFont(const String& fontName, float size)
{
	if(default_format_.font.face != fontName || default_format_.font.size != size){
		default_format_.font.face = fontName;
		default_format_.font.size = size;
		default_format_.font.bold = false;
		default_format_.font.italic = false;
		widget_->SetFlags(WidgetFlags_ContentChanged);
		charLocationsDirty_ = true;
		return true;
	}
	return false;
}

bool RichTextUI::SetFontSize(float size)
{
    // Initial font must be set
    if (default_format_.font.face.Length() == 0)
        return false;
    else
        return SetFont(default_format_.font.face, size);
}

void RichTextUI::DecodeToUnicode()
{
    unicodeText_.Clear();
    for (unsigned i = 0; i < text_.Length();)
        unicodeText_.Push(text_.NextUTF8Char(i));
}

void RichTextUI::SetText(const String& text)
{
    text_ = text;

    DecodeToUnicode();
    UpdateText();
}

void RichTextUI::SetTextAlignment(HorizontalAlignment align)
{
    if (align != default_format_.align)
    {
        default_format_.align = align;
        charLocationsDirty_ = true;
    }
}

void RichTextUI::SetRowSpacing(float spacing)
{
    if (spacing != rowSpacing_)
    {
        rowSpacing_ = Max(spacing, MIN_ROW_SPACING);
        UpdateText();
    }
}

void RichTextUI::SetWrapping(bool enable)
{
    if (enable != wrapping_)
    {
        wrapping_ = (enable) ? TextWrapping::WRAP_WORD : TextWrapping::WRAP_NONE;
        UpdateText();
		widget_->SetFlags(WidgetFlags_ContentChanged);
    }
}

void RichTextUI::SetAutoSize(bool autoSizing)
{
    autoSize_=autoSizing;
}

float RichTextUI::GetRowWidth(unsigned index) const
{
    return index < rowWidths_.Size() ? rowWidths_[index] : 0;
}

void RichTextUI::SetFontAttr(const ResourceRef& value)
{
    default_format_.font.face = value.name_;
    widget_->SetFlags(WidgetFlags_ContentChanged);
    charLocationsDirty_ = true;
}

ResourceRef RichTextUI::GetFontAttr() const
{
    return ResourceRef(Font::GetTypeStatic(), default_format_.font.face);
}

void RichTextUI::SetTextAttr(const String& value)
{
    text_ = value;
}

String RichTextUI::GetTextAttr() const
{
    return text_;
}

bool RichTextUI::FilterImplicitAttributes(XMLElement& dest) const
{
    if (!UIElement::FilterImplicitAttributes(dest))
        return false;

    if (!IsFixedWidth())
    {
        if (!RemoveChildXML(dest, "Size"))
            return false;
        if (!RemoveChildXML(dest, "Min Size"))
            return false;
        if (!RemoveChildXML(dest, "Max Size"))
            return false;
    }

    return true;
}

void RichTextUI::GetBatches(PODVector<UIBatch>& batches, PODVector<float>& vertexData, const IntRect& currentScissor)
{
    if(charLocationsDirty_ || widget_->IsFlagged(WidgetFlags_GeometryDirty)){
        UpdateText();
    }

    UIElement::GetBatches(batches, vertexData, currentScissor);

  // clear all quads
  widget_->Clear();

  RichWidgetText* text_renderer;
  RichWidgetImage* image_renderer;

  const IntVector2& screenPos = GetScreenPosition();
  int xoffset = 0, yoffset = 0;

  for (auto lit = lines_.Begin(); lit != lines_.End(); ++lit) {
    // adjust the size and offset of every block in a line
    TextLine* l = &(*lit);

    switch (l->align) {
    default:
    case HA_LEFT:
      xoffset = l->offset_x;
      break;
    case HA_CENTER:
      xoffset = (widget_->GetClipRegion().Width() - l->width) / 2;
      break;
    case HA_RIGHT:
      xoffset = widget_->GetClipRegion().Width() - l->width;
    }

    FontState fontstate;

    int line_max_height = 0;
    for (auto it = l->blocks.Begin(); it != l->blocks.End(); ++it) {
      if (it->type == TextBlock::BlockType_Image) {
        image_renderer = widget_->CacheWidgetBatch<RichWidgetImage>(it->text);
        //image_renderer->clearQuads();
        if (image_renderer->GetImageSource() != it->text)
          image_renderer->SetImageSource(it->text);

        if (it->image_width == 0)
          it->image_width = (float)widget_->GetClipRegion().Width();
        if (it->image_height == 0) {
          // height should be the line max height
          it->image_height = (float)line_max_height;
          // width should be auto calculated based on the texture size
          float aspect = image_renderer->GetImageAspect();
          it->image_width = it->image_height * aspect;
        }

        image_renderer->AddImage(Vector3((float)xoffset+screenPos.x_, (float)yoffset+screenPos.y_, 0.0f), it->image_width, it->image_height);
        line_max_height = Max((int)it->image_height, line_max_height);
        xoffset += (int)it->image_width;


      } else if (it->type == TextBlock::BlockType_Text) {
        fontstate = it->format.font;
        if (fontstate.face.Empty())
          fontstate.face = default_format_.font.face;
        if (fontstate.size <= 0)
          fontstate.size = default_format_.font.size;

        String bi = String(it->format.font.bold ? "b" : "") + String(it->format.font.italic ? "i" : "");
        if (!bi.Empty())
          bi = String(".") + bi;
        String id = fontstate.face + "." + ToString("%d", it->format.font.size) + bi;
        id.AppendWithFormat("%s.%d%s", fontstate.face.CString(), fontstate.size, bi.CString());
        text_renderer = widget_->CacheWidgetBatch<RichWidgetText>(id);
        // TODO: turn this back on when we have more fonts
        text_renderer->SetFont(fontstate.face, fontstate.size, fontstate.bold, fontstate.italic);
        //text_renderer->SetFont(default_font_state_.face, default_font_state_.size);

        text_renderer->AddText(it->text, Vector3((float)xoffset+screenPos.x_, (float)yoffset+screenPos.y_, 0.0f), it->format.color);
        if (text_renderer->GetFontFace()) {
          line_max_height = Max<int>((int)text_renderer->GetRowHeight(), line_max_height);
          xoffset += (int)text_renderer->CalculateTextExtents(it->text).x_;
        }
      }
    }
    yoffset += line_max_height + line_spacing_;
    Vector2 newContentSize(Max<float>(widget_->GetContentSize().x_, xoffset+screenPos.x_), (float)yoffset+screenPos.y_);
    widget_->SetContentSize(newContentSize);
  }
  //ResetTicker();

    widget_->Draw(this, batches, vertexData, currentScissor);
}

void RichTextUI::UpdateText(bool onResize)
{
  lines_.Clear();
  widget_->SetContentSize(Vector2::ZERO);

  Vector<TextBlock> markup_blocks;
  markup_blocks.Reserve(10);

  HTMLParser::Parse(text_, markup_blocks, default_format_);

  IntVector2 maxSize(0, 0);
  bool determineSize = ((GetSize().x_ == 0 && GetSize().y_ == 0) || autoSize_) ? true : false;

  TextLine line;
  if (!single_line_) {
    Vector<TextLine> markupLines;
    // for every new line in a block, create a new TextLine
    for (Vector<TextBlock>::ConstIterator it = markup_blocks.Begin(); it != markup_blocks.End(); ++it) {
      size_t posNewLine = 0;
      size_t posLast = 0;
      if ((posNewLine = it->text.Find("\n", posLast)) != String::NPOS) {
        while ((posNewLine = it->text.Find("\n", posLast)) != String::NPOS) {
          TextBlock block;
          block.text = it->text.Substring(posLast, posNewLine - posLast);
          block.format = it->format;
          line.blocks.Push(block);
          markupLines.Push(line);
          posLast = posNewLine + 1;
          line.blocks.Clear();
        }
        if (posLast < it->text.Length() - 1) {
          TextBlock block;
          block.text = it->text.Substring(posLast, it->text.Length() - posLast);
          block.format = it->format;
          line.blocks.Push(block);
        }
      } else {
        if (it->is_line_break) {
          line.blocks.Push(*it);
          markupLines.Push(line);
          line.blocks.Clear();
        }
        line.blocks.Push(*it);
        // update line alignment from this block if it is not left
        if (it->format.align != HA_LEFT)
          line.align = it->format.align;
      }
    }

    // make sure there's at least one line with text
    if (!line.blocks.Empty())
      markupLines.Push(line);

    IntRect actual_clip_region(0, 0, GetSize().x_, GetSize().y_);//widget_->GetClipRegion();
    if(determineSize == true){
        actual_clip_region = IntRect(0, 0, INT_MAX, INT_MAX);
    }else{
        widget_->SetClipRegion(actual_clip_region);
    }
    //GetScreenPosition() GetPosition
    // do the word wrapping and layout positioning
    if (wrapping_ == WRAP_WORD && actual_clip_region.Width()) {
      int layout_width = actual_clip_region.Width();
      int layout_height = actual_clip_region.Height();
      int layout_x = actual_clip_region.left_;
      int layout_y = actual_clip_region.top_;

      int draw_offset_x = layout_x;
      int draw_offset_y = layout_y;

      FontState fontstate;

      for (Vector<TextLine>::Iterator it = markupLines.Begin(); it != markupLines.End(); ++it) {
        TextLine* line = &*it;

        TextLine new_line;
        new_line.height = 0;
        new_line.width = 0;
        new_line.offset_x = layout_x;
        new_line.offset_y = 0;
        new_line.align = line->align;

        int maxRowHeight = 0;

        // reset the x offset of the current line
        draw_offset_x = layout_x;

        for (Vector<TextBlock>::Iterator bit = line->blocks.Begin(); bit != line->blocks.End(); ++bit) {
          TextBlock new_block;
          new_block.format = (*bit).format;

          if (bit->type == TextBlock::BlockType_Text) {
            size_t crpos;
            while ((crpos = bit->text.Find("\r")) != String::NPOS)
              bit->text.Erase(crpos, 1);

            Vector<String> words;
            splitwords(bit->text, words, splitter_chars, false);
            if (words.Empty())
              words.Push(bit->text);

            bool new_line_space = false;

            // bold-italic flags are passed as third asset name parameter
            String bi = String(bit->format.font.bold ? "b" : "") + String(bit->format.font.italic ? "i" : "");
            if (!bi.Empty())
              bi = String(".") + bi;

            // this block's text renderer
            String id;
            fontstate.face = bit->format.font.face;
            fontstate.size = bit->format.font.size;
            if (fontstate.face.Empty())
              fontstate.face = default_format_.font.face;
            if (fontstate.size <= 0)
              fontstate.size = default_format_.font.size;
            id.AppendWithFormat("%s.%d%s", fontstate.face.CString(), fontstate.size, bi.CString());
            RichWidgetText* text_renderer = widget_->CacheWidgetBatch<RichWidgetText>(id);
            text_renderer->SetFont(fontstate.face, fontstate.size);

            // for every word in this block do a check if there's enough space on the current line
            // Simple word wrap logic: if the space is enough, put the word on the current line, else go to the next line
            String the_word;
            for (Vector<String>::ConstIterator wit = words.Begin(); wit != words.End(); ++wit) {
              the_word = *wit;
              Vector2 wordsize = text_renderer->CalculateTextExtents(the_word);

              new_line.height = Max<int>((int)wordsize.y_, new_line.height);
              maxRowHeight = Max<int>(new_line.height, (int)text_renderer->GetRowHeight());

              bool needs_new_line = (draw_offset_x + wordsize.x_) > layout_width;
              bool is_wider_than_line = wordsize.x_ > layout_width;

              // Handle cases where this word can't be fit in a line
              while (is_wider_than_line && needs_new_line) {
                String fit_word = the_word;
                int width_remain = (int)wordsize.x_ - layout_width;
                assert(width_remain >= 0);
                // remove a char from this word until it can be fit in the current line
                Vector2 newwordsize;
                while (width_remain > 0 && !fit_word.Empty()) {
                  fit_word.Erase(fit_word.Length() - 1);
                  newwordsize = text_renderer->CalculateTextExtents(fit_word);
                  width_remain = (int)newwordsize.x_ - layout_width;
                }

                // prevent endless loops
                if (fit_word.Empty())
                  break;

                // append the fitting part of the word in the current line
                // and create a new line
                new_block.text.Append(fit_word);
                new_line.blocks.Push(new_block);
                draw_offset_x += (int)newwordsize.x_;
                new_line.width = draw_offset_x;
                new_line.offset_y = draw_offset_y;
                lines_.Push(new_line);
                // create a new empty line
                new_line.blocks.Clear();
                new_block.text.Clear();
                draw_offset_x = layout_x;
                draw_offset_y += new_line.height;

                the_word = the_word.Substring(fit_word.Length());

                wordsize = text_renderer->CalculateTextExtents(the_word);
                draw_offset_x += (int)wordsize.x_;
                is_wider_than_line = wordsize.x_ > layout_width;

                if (!is_wider_than_line) {
                  needs_new_line = false;
                  // the leftovers from the_word will be added to the line below
                  break;
                }
              }

              // carry the whole word on a new line
              if (needs_new_line) {
                new_line.width = draw_offset_x;
                new_line.offset_y = draw_offset_y;
                new_line.blocks.Push(new_block);
                lines_.Push(new_line);
                // create next empty line
                new_line.blocks.Clear();
                // add current text to a block
                new_block.text.Clear();
                draw_offset_x = layout_x;
                draw_offset_y += new_line.height;
                if (the_word == " " || the_word == "\t") {
                  // mark as space
                  new_line_space = true;
                  continue;
                }
              }

              if (new_line_space) {
                if (the_word != " " && the_word != "\t")
                  new_line_space = false;
                else
                  continue;
              }
              new_block.text.Append(the_word);
              draw_offset_x += (int)wordsize.x_;
            }

            new_line.blocks.Push(new_block);
          } else {
            // if the block is iconic (image, video, etc)
            new_block = (*bit); // copy the block
            if (new_block.is_visible) {
              RichWidgetImage* image_renderer = widget_->CacheWidgetBatch<RichWidgetImage>(new_block.text);
              image_renderer->SetImageSource(new_block.text);
              if (new_block.image_height == 0) {
                float aspect = image_renderer->GetImageAspect();
                if (aspect == 0.f)
                  aspect = 1.0f;

                // fit by height
                //new_block.image_height = clip_region_.height();
                //new_block.image_width = new_block.image_height * aspect;

                // fit by width
                //new_block.image_width = (float)(new_block.image_width > 0 ? new_block.image_width : widget_->GetClipRegion().Width());
                new_block.image_width = (float)(new_block.image_width > 0 ? new_block.image_width : actual_clip_region.Width());
                new_block.image_height = new_block.image_width / aspect;
              }

              if (new_block.image_width == 0) {
                //new_block.image_width = (float)widget_->GetClipRegion().Width();
                new_block.image_width = (float)actual_clip_region.Width();
              }
              new_line.blocks.Push(new_block);

              draw_offset_x += (int)new_block.image_width;
              new_line.height = Max((int)new_block.image_height, new_line.height);
            }
          }

          new_line.width = draw_offset_x;
          new_line.offset_y = draw_offset_y;
          draw_offset_y += new_line.height;
        }

        maxSize.x_ = Max(draw_offset_x, maxSize.x_);
        maxSize.y_ = maxSize.y_ + Max(new_line.height, maxRowHeight);//Max(draw_offset_y, maxSize.y_);

        lines_.Push(new_line);
        new_line.blocks.Clear();
      }
    } else {
      // in case there's no word wrapping or the layout has no width
      lines_ = markupLines;
    }


  } else {
    // Single line...
    // replace /n/r with empty space
    for (auto i = markup_blocks.Begin(); i != markup_blocks.End(); i++) {
      size_t crpos;
      while ((crpos = find_first_of(i->text, "\n\r")) != String::NPOS) {
        i->text.Replace(crpos, 1, " ");
      }
      // TODO: single line doesn't get images when width or height = 0
      line.blocks.Push(*i);
    }
    lines_.Push(line);
  }


    if(determineSize){// && !IsFixedSize()){
        if(!maxSize.y_){
            maxSize.y_ = GetFontSize();
        }
        if(GetWidth() != maxSize.x_ || GetHeight() != maxSize.y_){
            widget_->SetClipRegion(IntRect(0, 0, maxSize.x_, maxSize.y_));
            SetFixedSize(maxSize);
        }
    }

    widget_->ClearFlags(WidgetFlags_ContentChanged);
    widget_->SetFlags(WidgetFlags_GeometryDirty);
    charLocationsDirty_ = false;
}

} // namespace Urho3D
