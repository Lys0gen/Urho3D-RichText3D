#ifndef __RICH_FONT_PROVIDER_H__
#define __RICH_FONT_PROVIDER_H__
#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/UI/Font.h>
#include "rich_batch_text.h"

namespace Urho3D {

/// RichTextFontRequest
URHO3D_EVENT(E_RICHTEXT_FONT_REQUEST, RichTextFontRequest) {
  URHO3D_PARAM(P_RICHWIDGETTEXT, TextWidget);      // RichWidgetText
  URHO3D_PARAM(P_ID, Id); // unsigned
  URHO3D_PARAM(P_FONTNAME, Name);      // String
  URHO3D_PARAM(P_BOLD, Bold); // bool
  URHO3D_PARAM(P_ITALIC, Italic); // bool
}

/// A proxy object for resolving font name (string) to Font (resource)
class RichFontProvider : public Object {
  URHO3D_OBJECT(RichFontProvider, Object)
public:
  RichFontProvider(Context* context);
  ~RichFontProvider() override;

  /// Request a font by name, the actual implementation must call CompleteRequest()
  void RequestFont(RichWidgetText* textwidget, const String& fontname, bool bold, bool italic);
  /// Cancel request
  void CancelRequest(RichWidgetText* textwidget);
  /// Complete request, if font is nullptr default font will be used 
  void CompleteRequest(unsigned request_id, const String& filename);

  void AddFontMapping(const String& name, bool bold, bool italic, const String& font_resource_name);
  void RemoveFontMapping(const String& name, bool bold, bool italic);
  void ClearFontMapping();
private:
  struct FontParams {
    String name;
    bool bold;
    bool italic;
  };

  HashMap<StringHash, Pair<WeakPtr<RichWidgetText>, FontParams>> pending_requests_;

  struct RichFontDescription {
    /// Base font name (eg. Anonymous Pro)
    String name;
    /// Is the font bold.
    bool bold;
    /// Is the font italic.
    bool italic;
    /// The resource font name (eg. Fonts/Anonymous Pro.ttf)
    String font_resource_name;
  };

  Vector<RichFontDescription> font_mapping_;
};

} // namespace Urho3D

#endif
