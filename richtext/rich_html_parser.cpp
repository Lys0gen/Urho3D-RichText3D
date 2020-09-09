#include "rich_html_parser.h"

namespace Urho3D
{

namespace
{

typedef struct
{
    const char* name;
    const Color color;
} ColorEntry;

inline Color ParseHTMLColor(const String& str)
{
    static const ColorEntry color_table[] = {
      { "red", Color::RED },
      { "green", Color::GREEN },
      { "blue", Color::BLUE },
      { "black", Color::BLACK },
      { "white", Color::WHITE },
      { "transparent", Color::TRANSPARENT_BLACK },
      { "yellow", Color::YELLOW },
      { "cyan", Color::CYAN },
      { "magenta", Color::MAGENTA },
      { "gray", Color::GRAY },
      { "orange", Color(1.0f, 0.647f, 0.0f) },
      // TODO: Add more colors
      { NULL, Color() },
    };

    unsigned char R = 0, G = 0, B = 0, A = 0;

    String color = str.ToLower();

    for (int i = 0; color_table[i].name; i++)
    {
        const ColorEntry* entry = &color_table[i];
        if (color.Compare(entry->name) == 0)
            return entry->color;
    }

    if (color.Length() == 7 || color.Length() == 9)
    {
        if (color.Find("#") == 0)
        {
            R = (unsigned char)strtoul(str.Substring(1, 2).CString(), 0, 16);
            G = (unsigned char)strtoul(str.Substring(3, 2).CString(), 0, 16);
            B = (unsigned char)strtoul(str.Substring(5, 2).CString(), 0, 16);
            A = str.Length() == 7 ? 255 : (unsigned char)strtoul(str.Substring(7, 2).CString(), 0, 16);
        }
    }

    return Color(R / 255.0f, G / 255.0f, B / 255.0f, A / 255.0f);
}

void UnescapeQuotes(String& s)
{
  if (s.StartsWith("\"") && s.EndsWith("\""))
  {
      s.Erase(0);
      s.Erase(s.Length() - 1);
  }
}

StringVector QuotedSplit(const char* str, char separator) {
  Vector<String> ret;
  const char* strEnd = str + String::CStringLength(str);

  bool in_quotes = false;

  for (const char* splitEnd = str; splitEnd != strEnd; ++splitEnd)
  {
    if (*splitEnd == '\"')
      in_quotes = !in_quotes;
    else if (!in_quotes && *splitEnd == separator)
    {
      const ptrdiff_t splitLen = splitEnd - str;
      if (splitLen > 0)
        ret.Push(String(str, splitLen));
      str = splitEnd + 1;
    }
  }

  const ptrdiff_t splitLen = strEnd - str;
  if (splitLen > 0 && !in_quotes)
    ret.Push(String(str, splitLen));

  return ret;
}

} // namespace

/// Supported tags:
///  <br> - line break
///  <b></b> - bold
///  <i></i> - italic
///  <u></u> - underlined
///  <sup></sup> - superscript
///  <sub></sub> - subscript
///  <align=left|right|center></align>
///  <color=#FF800000></color> - change text color, supports 24/32 bit hex and color names
///  <size=14></size> - change text size in pixels
///  <font=Fonts/Annonymous Pro.ttf></font> - change text font
///  <img src=image.png width=320 height=240 /> - embed an image
///  <plugin type=typename key=val ... /> - embed a plugin
///  TODO: <quad material=material.xml width=10 height=10 x=10 y=10 />
void HTMLParser::Parse(const String& text, Vector<TextBlock>& blocks, const BlockFormat& default_block_format)
{
    unsigned pos = 0, last_tag_end = 0;
    unsigned tag_begin = 0, tag_end = 0;
    bool closing_tag = false;
    String tag;

    Vector<BlockFormat> stack;
    TextBlock block;
    BlockFormat block_format = default_block_format;

    block.format = block_format;

    pos = text.Find('<', pos, false);

    // in case there's no markup at all, copy everything as a block
    if (pos == String::NPOS) {
        block.text = text;
        blocks.Push(block);
        return;
    }

    while (String::NPOS != (pos = text.Find('<', pos, false)))
    {
        tag_end = text.Find('>', pos + 1, false);
        // skip <>
        if (tag_end == pos + 1)
        {
            pos += 1;
            continue;
        }

        // stop if there's no closing tag or text ends
        if (pos + 1 >= text.Length() || tag_end == String::NPOS)
            break;

        tag_begin = pos;
        closing_tag = text[pos + 1] == '/';
        pos += closing_tag ? 2 : 1;
        tag = text.Substring(pos, tag_end - pos);
        if (last_tag_end != tag_begin)
        {
            block.text = text.Substring(last_tag_end, tag_begin - last_tag_end);
            // push the current block everytime new block appears
            blocks.Push(block);
        }
        
        if (tag == "b")
        {
            block.format.font.bold = !closing_tag;
        }
        else if (tag == "i")
        {
            block.format.font.italic = !closing_tag;
        }
        else if (tag == "u")
        {
          block.format.underlined = !closing_tag;
        }
        else if (tag == "sub")
        {
          block.format.subscript = !closing_tag;
        }
        else if (tag == "sup")
        {
          block.format.superscript = !closing_tag;
        }
        else if (tag.StartsWith("align"))
        {
          auto tokens = tag.Split('=');
          if (tokens.Size() >= 2)
          {
            UnescapeQuotes(tokens[1]);
            if (tokens[1] == "center")
              block.format.align = HA_CENTER;
            else if (tokens[1] == "right")
              block.format.align = HA_RIGHT;
          }
        }
        else if (tag == "br")
        {
            TextBlock br;
            br.type = TextBlock::BlockType_Text;
            br.is_line_break = true;
            blocks.Push(br);
        }
        else if (tag.StartsWith("color"))
        {
            if (!closing_tag)
            {
                stack.Push(block.format);
                auto tokens = tag.Split('=');
                if (tokens.Size() >= 2)
                {
                    UnescapeQuotes(tokens[1]);
                    block.format.color = ParseHTMLColor(tokens[1]);
                }
            } else {
                // pop font style from stack
                if (!stack.Empty())
                {
                    block.format = stack.Back();
                    stack.Pop();
                } else
                    block.format = default_block_format;
            }
        }
        else if (tag.StartsWith("size"))
        {
            if (!closing_tag)
            {
                stack.Push(block.format);
                auto tokens = tag.Split('=');
                if (tokens.Size() >= 2)
                {
                    UnescapeQuotes(tokens[1]);
                    block.format.font.size = ToInt(tokens[1]);
                }
            } else {
                // pop font style from stack
                if (!stack.Empty())
                {
                    block.format = stack.Back();
                    stack.Pop();
                } else
                    block.format = default_block_format;
            }
        }
        else if (tag.StartsWith("font "))
        {
            if (!closing_tag)
            {
                stack.Push(block.format);
                auto font_tokens = QuotedSplit(tag.CString(), ' ');
                for (auto& t : font_tokens)
                {
                  auto tokens = t.Split('=');
                  if (tokens.Size() != 2)
                    continue;

                  const String& name = tokens[0];
                  String& value = tokens[1];
                  UnescapeQuotes(value);
                  if (name == "face") {
                    block.format.font.face = value;
                  } else if (name == "color") {
                    block.format.color = ParseHTMLColor(value);
                  } else if (name == "size") {
                    block.format.font.size = ToInt(value);
                  }
                }
            }
            else
            {
                // pop font style from stack
                if (!stack.Empty()) {
                    block.format = stack.Back();
                    stack.Pop();
                } else
                    block.format = default_block_format;
            }
        }
        else if (tag.StartsWith("font"))
        {
            stack.Push(block.format);
            auto tokens = tag.Split('=');
            if (tokens.Size() == 2)
            {
                UnescapeQuotes(tokens[1]);
                block.format.font.face = tokens[1];
            }
            else
            {
                // pop font style from stack
                if (!stack.Empty()) {
                    block.format = stack.Back();
                    stack.Pop();
                } else
                    block.format = default_block_format;
            }
        }
        else if (tag.StartsWith("img"))
        {
            TextBlock img;
            img.type = TextBlock::BlockType_Image;
            closing_tag = true;
            auto img_tokens = tag.Split(' ');
            for (auto& t : img_tokens)
            {
                auto tokens = t.Split('=');
                if (tokens.Size() != 2)
                    continue;

                const String& name = tokens[0];
                String& value = tokens[1];
                UnescapeQuotes(value);
                if (name == "width")
                    img.image_width = ToFloat(value);
                else if (name == "height")
                    img.image_height = ToFloat(value);
                else if (name == "src")
                    img.text = value;
            }
            blocks.Push(img);
        }
        else if (tag.StartsWith("quad"))
        {
            TextBlock quad;
            quad.type = TextBlock::BlockType_Image;
            closing_tag = true;
            auto img_tokens = tag.Split(' ');
            for (auto& t : img_tokens)
            {
                auto tokens = t.Split('=');
                if (tokens.Size() != 2)
                    continue;
                const String& name = tokens[0];
                String& value = tokens[1];
                UnescapeQuotes(value);
                if (name == "width")
                    quad.image_width = ToFloat(value);
                else if (name == "height")
                    quad.image_height = ToFloat(value);
                else if (name == "material")
                    quad.text = value;
            }
            blocks.Push(quad);
        }
        else if (tag.StartsWith("plugin"))
        {
          TextBlock plugin;
          plugin.type = TextBlock::BlockType_Plugin;
          plugin.text = tag.Substring(tag.Find(" ")).Trimmed();
          closing_tag = true;
          blocks.Push(plugin);
        }

        pos = tag_end + 1;
        last_tag_end = tag_end + 1;
    }
    
    // copy anything after the last tag
    if (last_tag_end < text.Length())
    {
        TextBlock block;
        block.format = default_block_format;
        block.text = text.Substring(last_tag_end, text.Length() - last_tag_end);
        blocks.Push(block);
    }

    // append blocks that are not closed properly
    /*if (!closing_tag && current_block && !current_block->text.Empty())
    {
        blocks.Push(block);
    }*/
}

} // namespace Urho3D