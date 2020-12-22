#ifndef __RICH_HTML_PARSER_H__
#define __RICH_HTML_PARSER_H__
#pragma once

#include "rich_text3d.h"

namespace Urho3D
{

/// An utility class that parses RichText markup.
class HTMLParser
{
public:
    /// Parse the text and outputs the text as TextBlock array.
    static void Parse(const String& text, Vector<TextBlock>& blocks, const BlockFormat& default_block_format);
};

} // namespace Urho3D

#endif
