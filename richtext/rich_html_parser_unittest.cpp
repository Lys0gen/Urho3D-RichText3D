#include "gtest/gtest.h"

#include "rich_html_parser.h"

#if defined(TARGET_WINDOWS)
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "version.lib")
#endif

TEST(RichTextHTMLParser, Html4FontStyle) {
  Urho3D::Vector<Urho3D::TextBlock> blocks;

  Urho3D::HTMLParser::Parse("<font size=8 color=red face=fontface>styled text</font>", blocks, Urho3D::BlockFormat());
  ASSERT_EQ(blocks.Size(), 1);
  EXPECT_EQ(blocks[0].text, "styled text");
  EXPECT_EQ(blocks[0].type, TextBlock::BlockType_Text);
  EXPECT_EQ(blocks[0].format.font.size, 8);
  EXPECT_EQ(blocks[0].format.font.face, "fontface");
  EXPECT_EQ(blocks[0].format.color, Urho3D::Color::RED);
}

TEST(RichTextHTMLParser, SingleBlock) {
  Urho3D::Vector<Urho3D::TextBlock> blocks;

  Urho3D::HTMLParser::Parse("single block", blocks, Urho3D::BlockFormat());
  ASSERT_EQ(blocks.Size(), 1);

  EXPECT_EQ(blocks[0].text, "single block");
  EXPECT_EQ(blocks[0].type, TextBlock::BlockType_Text);
  EXPECT_FALSE(blocks[0].is_line_break);
  EXPECT_TRUE(blocks[0].is_visible);
  EXPECT_FALSE(blocks[0].format.font.bold);
  EXPECT_FALSE(blocks[0].format.font.italic);
  EXPECT_FALSE(blocks[0].format.underlined);
  EXPECT_FALSE(blocks[0].format.striked);
  EXPECT_FALSE(blocks[0].format.subscript);
  EXPECT_FALSE(blocks[0].format.superscript);
}

TEST(RichTextHTMLParser, MultipleBlocks) {
  Urho3D::Vector<Urho3D::TextBlock> blocks;

  Urho3D::HTMLParser::Parse("first<br>second<br>third", blocks, Urho3D::BlockFormat());

  ASSERT_EQ(blocks.Size(), 5);

  EXPECT_STREQ(blocks[0].text.CString(), "first");
  EXPECT_TRUE(blocks[1].is_line_break);
  EXPECT_STREQ(blocks[2].text.CString(), "second");
  EXPECT_TRUE(blocks[3].is_line_break);
  EXPECT_STREQ(blocks[4].text.CString(), "third");
}

TEST(RichTextHTMLParser, Nesting) {
  Urho3D::Vector<Urho3D::TextBlock> blocks;

  Urho3D::HTMLParser::Parse("outer block 1<b>inner block 2<i>nested block 3</i></b>", blocks, Urho3D::BlockFormat());

  ASSERT_EQ(blocks.Size(), 3);

  EXPECT_STREQ(blocks[0].text.CString(), "outer block 1");
  EXPECT_STREQ(blocks[1].text.CString(), "inner block 2");
  EXPECT_TRUE(blocks[1].format.font.bold);
  EXPECT_STREQ(blocks[2].text.CString(), "nested block 3");
  EXPECT_TRUE(blocks[2].format.font.bold && blocks[2].format.font.italic);
}

TEST(RichTextHTMLParser, FormatColor) {
  Urho3D::Vector<Urho3D::TextBlock> blocks;

  Urho3D::HTMLParser::Parse("default<color=red>red</color><color=blue>blue</color>default 2", blocks, Urho3D::BlockFormat());

  ASSERT_EQ(blocks.Size(), 4);

  EXPECT_STREQ(blocks[0].text.CString(), "default");
  EXPECT_STREQ(blocks[1].text.CString(), "red");
  EXPECT_EQ(blocks[1].format.color, Urho3D::Color::RED);
  EXPECT_STREQ(blocks[2].text.CString(), "blue");
  EXPECT_EQ(blocks[2].format.color, Urho3D::Color::BLUE);
  EXPECT_STREQ(blocks[3].text.CString(), "default 2");

  // nesting
  blocks.Clear();
  Urho3D::HTMLParser::Parse("default<color=red>red<color=blue>blue</color></color>default 2", blocks, Urho3D::BlockFormat());
  ASSERT_EQ(blocks.Size(), 4);

  EXPECT_STREQ(blocks[0].text.CString(), "default");
  EXPECT_STREQ(blocks[1].text.CString(), "red");
  EXPECT_EQ(blocks[1].format.color, Urho3D::Color::RED);
  EXPECT_STREQ(blocks[2].text.CString(), "blue");
  EXPECT_EQ(blocks[2].format.color, Urho3D::Color::BLUE);
  EXPECT_STREQ(blocks[3].text.CString(), "default 2");

  // default color after tag closed
  blocks.Clear();
  Urho3D::BlockFormat white_font;
  white_font.color = Color::WHITE;
  Urho3D::HTMLParser::Parse("<color=yellow>y</color>2  2312 <color=blue>b</color>", blocks, white_font);
  ASSERT_EQ(blocks.Size(), 3);
  EXPECT_STREQ(blocks[0].text.CString(), "y");
  EXPECT_EQ(blocks[0].format.color, Urho3D::Color::YELLOW);
  EXPECT_STREQ(blocks[1].text.CString(), "2  2312 ");
  EXPECT_EQ(blocks[1].format.color, Urho3D::Color::WHITE);
  EXPECT_STREQ(blocks[2].text.CString(), "b");
  EXPECT_EQ(blocks[2].format.color, Urho3D::Color::BLUE);

}

TEST(RichTextHTMLParser, FormatSize) {
  Urho3D::Vector<Urho3D::TextBlock> blocks;

  Urho3D::BlockFormat default_state;
  default_state.font.size = 10;

  Urho3D::HTMLParser::Parse("size 10<size=12>size 12</size><size=14>size 14</size>size 10 2", blocks, default_state);

  ASSERT_EQ(blocks.Size(), 4);
  EXPECT_STREQ(blocks[0].text.CString(), "size 10");
  EXPECT_EQ(blocks[0].format.font.size, 10);
  EXPECT_STREQ(blocks[1].text.CString(), "size 12");
  EXPECT_EQ(blocks[1].format.font.size, 12);
  EXPECT_STREQ(blocks[2].text.CString(), "size 14");
  EXPECT_EQ(blocks[2].format.font.size, 14);
  EXPECT_STREQ(blocks[3].text.CString(), "size 10 2");
  EXPECT_EQ(blocks[3].format.font.size, 10);
}

TEST(RichTextHTMLParser, FormatFace) {
  Urho3D::Vector<Urho3D::TextBlock> blocks;

  Urho3D::BlockFormat default_state;
  default_state.font.face = "default";

  Urho3D::HTMLParser::Parse("face default<font=first>face first</font><font=second>face second</font>face default 2", blocks, default_state);

  ASSERT_EQ(blocks.Size(), 4);
  EXPECT_STREQ(blocks[0].text.CString(), "face default");
  EXPECT_STREQ(blocks[0].format.font.face.CString(), "default");
  EXPECT_STREQ(blocks[1].text.CString(), "face first");
  EXPECT_STREQ(blocks[1].format.font.face.CString(), "first");
  EXPECT_STREQ(blocks[2].text.CString(), "face second");
  EXPECT_STREQ(blocks[2].format.font.face.CString(), "second");
  EXPECT_STREQ(blocks[3].text.CString(), "face default 2");
  EXPECT_STREQ(blocks[3].format.font.face.CString(), "default");
}

TEST(RichTextHTMLParser, Image) {
  Urho3D::Vector<Urho3D::TextBlock> blocks;

  Urho3D::HTMLParser::Parse("text <img src=image_source.png width=800 height=600> text 2", blocks, Urho3D::BlockFormat());

  ASSERT_EQ(blocks.Size(), 3);
  EXPECT_STREQ(blocks[0].text.CString(), "text ");

  EXPECT_EQ(blocks[1].type, TextBlock::BlockType_Image);
  EXPECT_STREQ(blocks[1].text.CString(), "image_source.png");
  EXPECT_FLOAT_EQ(blocks[1].image_width, 800.0f);
  EXPECT_FLOAT_EQ(blocks[1].image_height, 600.0f);
  EXPECT_STREQ(blocks[2].text.CString(), " text 2");
}

TEST(RichTextHTMLParser, Quad) {
  Urho3D::Vector<Urho3D::TextBlock> blocks;

  Urho3D::HTMLParser::Parse("text <quad material=material.xml width=800 height=600 x=10 y=20> text 2", blocks, Urho3D::BlockFormat());
  ASSERT_EQ(blocks.Size(), 3);
  EXPECT_STREQ(blocks[0].text.CString(), "text ");

  EXPECT_EQ(blocks[1].type, TextBlock::BlockType_Image);
  EXPECT_STREQ(blocks[1].text.CString(), "material.xml");
  EXPECT_FLOAT_EQ(blocks[1].image_width, 800.0f);
  EXPECT_FLOAT_EQ(blocks[1].image_height, 600.0f);
  EXPECT_STREQ(blocks[2].text.CString(), " text 2");
}

TEST(RichTextHTMLParser, Plugin) {
  Urho3D::Vector<Urho3D::TextBlock> blocks;

  Urho3D::HTMLParser::Parse("text <plugin type=video width=800 height=600> text 2", blocks, Urho3D::BlockFormat());
  ASSERT_EQ(blocks.Size(), 3);

  EXPECT_EQ(blocks[0].type, TextBlock::BlockType_Text);
  EXPECT_STREQ(blocks[0].text.CString(), "text ");
  EXPECT_EQ(blocks[1].type, TextBlock::BlockType_Plugin);
  EXPECT_STREQ(blocks[1].text.CString(), "type=video width=800 height=600");

  EXPECT_STREQ(blocks[2].text.CString(), " text 2");

  blocks.Clear();
  Urho3D::HTMLParser::Parse("empty <plugin>", blocks, Urho3D::BlockFormat());
  ASSERT_EQ(blocks.Size(), 2);
  EXPECT_EQ(blocks[0].type, TextBlock::BlockType_Text);
  EXPECT_STREQ(blocks[0].text.CString(), "empty ");
  EXPECT_EQ(blocks[1].type, TextBlock::BlockType_Plugin);
}

TEST(RichTextHTMLParser, HTMLCompatibilityTags) {
  Urho3D::Vector<Urho3D::TextBlock> blocks;

  Urho3D::HTMLParser::Parse("text <font color=\"red\" size=\"13\" face=\"Roboto\">formatted</font>", blocks, Urho3D::BlockFormat());
  ASSERT_EQ(blocks.Size(), 2);

  EXPECT_EQ(blocks[0].type, TextBlock::BlockType_Text);
  EXPECT_STREQ(blocks[0].text.CString(), "text ");
  EXPECT_EQ(blocks[1].type, TextBlock::BlockType_Text);
  EXPECT_STREQ(blocks[1].text.CString(), "formatted");
  EXPECT_EQ(blocks[1].format.color, Urho3D::Color::RED);
  EXPECT_EQ(blocks[1].format.font.size, 13);
  EXPECT_STREQ(blocks[1].format.font.face.CString(), "Roboto");
}

TEST(RichTextHTMLParser, FontNameWithSpace) {
  Urho3D::Vector<Urho3D::TextBlock> blocks;

  Urho3D::HTMLParser::Parse("text <font=\"Anonymous Pro\">formatted</font>", blocks, Urho3D::BlockFormat());
  ASSERT_EQ(blocks.Size(), 2);

  EXPECT_EQ(blocks[0].type, TextBlock::BlockType_Text);
  EXPECT_STREQ(blocks[0].text.CString(), "text ");
  EXPECT_EQ(blocks[1].type, TextBlock::BlockType_Text);
  EXPECT_STREQ(blocks[1].text.CString(), "formatted");
  EXPECT_STREQ(blocks[1].format.font.face.CString(), "Anonymous Pro");

  blocks.Clear();
  Urho3D::HTMLParser::Parse("text <font face=\"Anonymous Pro\">formatted</font>", blocks, Urho3D::BlockFormat());
  ASSERT_EQ(blocks.Size(), 2);

  EXPECT_EQ(blocks[0].type, TextBlock::BlockType_Text);
  EXPECT_STREQ(blocks[0].text.CString(), "text ");
  EXPECT_EQ(blocks[1].type, TextBlock::BlockType_Text);
  EXPECT_STREQ(blocks[1].text.CString(), "formatted");
  EXPECT_STREQ(blocks[1].format.font.face.CString(), "Anonymous Pro");
}

TEST(RichTextHTMLParser, BugHTMLFriendsTitle) {
  Urho3D::Vector<Urho3D::TextBlock> blocks;

  Urho3D::HTMLParser::Parse("<font face=\"GF@Gloria Hallelujah\">F<color=red>·</color>R<color=yellow>·</color>I<color=blue>·</color>E<color=#6e0000>·</color>N<color=white>·</color>D<color=#00004a>·</color>S</font>",
    blocks, Urho3D::BlockFormat());
  ASSERT_EQ(blocks.Size(), 13);

  
  EXPECT_STREQ(blocks[0].format.font.face.CString(), "GF@Gloria Hallelujah");
  EXPECT_STREQ(blocks[1].format.font.face.CString(), "GF@Gloria Hallelujah");
  EXPECT_STREQ(blocks[2].format.font.face.CString(), "GF@Gloria Hallelujah");
  EXPECT_STREQ(blocks[3].format.font.face.CString(), "GF@Gloria Hallelujah");
  EXPECT_STREQ(blocks[4].format.font.face.CString(), "GF@Gloria Hallelujah");
  EXPECT_STREQ(blocks[5].format.font.face.CString(), "GF@Gloria Hallelujah");
  EXPECT_STREQ(blocks[6].format.font.face.CString(), "GF@Gloria Hallelujah");
  EXPECT_STREQ(blocks[7].format.font.face.CString(), "GF@Gloria Hallelujah");
  EXPECT_STREQ(blocks[8].format.font.face.CString(), "GF@Gloria Hallelujah");
  EXPECT_STREQ(blocks[9].format.font.face.CString(), "GF@Gloria Hallelujah");
  EXPECT_STREQ(blocks[10].format.font.face.CString(), "GF@Gloria Hallelujah");
  EXPECT_STREQ(blocks[11].format.font.face.CString(), "GF@Gloria Hallelujah");
  EXPECT_STREQ(blocks[12].format.font.face.CString(), "GF@Gloria Hallelujah");  
}
