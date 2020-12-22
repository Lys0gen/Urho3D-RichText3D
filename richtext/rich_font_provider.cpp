#include "rich_font_provider.h"
#include "rich_widget.h"
#include <Urho3D/Resource/ResourceCache.h>
//#include "../base_application.h"
//#include "core/logger.h"

namespace Urho3D {

RichFontProvider::RichFontProvider(Context* context)
  : Object(context) {

}

RichFontProvider::~RichFontProvider() {

}

void RichFontProvider::RequestFont(RichWidgetText* textwidget, const String& fontname, bool bold, bool italic) {
  ResourceCache* cache = GetSubsystem<ResourceCache>();
  String resolved_fontname = fontname;

  String lowercase_fontname = fontname.ToLower();
  for (auto it = font_mapping_.Begin(); it != font_mapping_.End(); ++it) {
    if (it->name == lowercase_fontname && it->bold == bold && it->italic == italic) {
      resolved_fontname = it->font_resource_name;
      break;
    }
  }

  if (!resolved_fontname.Empty() && cache->Exists(resolved_fontname)) {
    auto font = cache->GetResource<Font>(resolved_fontname);
    if (font) {
      textwidget->SetFontResource(font);
      return;
    }
  }

  StringHash id(fontname);
  pending_requests_[id] = Pair<WeakPtr<RichWidgetText>, FontParams>(WeakPtr<RichWidgetText>(textwidget), {fontname, bold, italic});

  using namespace RichTextFontRequest;
  VariantMap& eventData = GetEventDataMap();
  eventData[P_RICHWIDGETTEXT] = textwidget;
  eventData[P_ID] = id.Value();
  eventData[P_FONTNAME] = fontname;
  eventData[P_BOLD] = bold;
  eventData[P_ITALIC] = italic;
  SendEvent(E_RICHTEXT_FONT_REQUEST, eventData);
}

void RichFontProvider::CancelRequest(RichWidgetText* textwidget) {
  for (auto it = pending_requests_.Begin(); it != pending_requests_.End();) {
    if (it->second_.first_ == textwidget)
      it = pending_requests_.Erase(it);
    else
      ++it;
  }
}

void RichFontProvider::CompleteRequest(unsigned request_id, const String& filename) {
  String default_fontname;

  HashMap<StringHash, Pair<WeakPtr<RichWidgetText>, FontParams>>::Iterator req_it;
  while ((req_it = pending_requests_.Find(StringHash(request_id))) != pending_requests_.End()) {
    if (!filename.Empty()) {
      if (req_it->second_.first_) {
        auto font = new Urho3D::Font(context_);
        font->SetName(filename);
        if (!font->LoadFile(filename)) {
//          WLOG_DEBUG(kAppLogTag) << "Failed to load font " << req_it->second_.second_.name.CString();
          delete font;
          font = nullptr;
        } else {
//          WLOG_DEBUG(kAppLogTag) << "Loaded font " << req_it->second_.second_.name.CString();
          req_it->second_.first_->SetFontResource(font);
        }
      } else {
 //       WLOG_DEBUG(kAppLogTag) << "Missing widget " << req_it->second_.second_.name.CString();
      }
      AddFontMapping(req_it->second_.second_.name, req_it->second_.second_.bold, req_it->second_.second_.italic, filename);
    } else {
      // find first font with matching bold/italic
      for (auto it = font_mapping_.Begin(); it != font_mapping_.End(); ++it) {
        if (it->bold == req_it->second_.second_.bold && it->italic == req_it->second_.second_.italic) {
          default_fontname = it->font_resource_name;
          break;
        }
      }

      ResourceCache* cache = GetSubsystem<ResourceCache>();
      // Use default font
      if (!default_fontname.Empty() && cache->Exists(default_fontname)) {
        auto font = cache->GetResource<Font>(default_fontname);
        if (font) {
          if (req_it->second_.first_)
            req_it->second_.first_->SetFontResource(font);
          AddFontMapping(req_it->second_.second_.name, req_it->second_.second_.bold, req_it->second_.second_.italic, font->GetName().CString());
        }
      }
    }
    // mark as content changed, the parent widget will rebuild
    if (req_it->second_.first_)
      req_it->second_.first_->GetParentWidget()->SetFlags(WidgetFlags_ContentChanged);
    pending_requests_.Erase(req_it);
  }
}

void RichFontProvider::AddFontMapping(const String& name, bool bold, bool italic, const String& font_resource_name) {
  RichFontDescription desc;
  desc.name = name.ToLower();
  desc.bold = bold;
  desc.italic = italic;
  desc.font_resource_name = font_resource_name;
  font_mapping_.Push(desc);
}

void RichFontProvider::RemoveFontMapping(const String& name, bool bold, bool italic) {
  String lowercase_fontname = name.ToLower();
  for (auto it = font_mapping_.Begin(); it != font_mapping_.End();) {
    if (it->name == lowercase_fontname && it->bold == bold && it->italic == italic) {
      it = font_mapping_.Erase(it);
    } else
      ++it;
  }
}

void RichFontProvider::ClearFontMapping() {
  font_mapping_.Clear();
}

} // namespace Urho3D
