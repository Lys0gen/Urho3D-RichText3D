#include "rich_image_provider.h"
#include "rich_widget.h"

namespace Urho3D {

RichImageProvider::RichImageProvider(Context* context)
 : Object(context) {

}

RichImageProvider::~RichImageProvider() {

}

void RichImageProvider::RequestImageResource(RichWidgetImage* image, const String& url) {
  String url_clean = url;
  while (url_clean.Contains('"'))
    url_clean.Replace("\"", "");

  pending_requests_[StringHash(url_clean)] = image;

  using namespace RichTextImageRequest;
  VariantMap& eventData = GetEventDataMap();
  eventData[P_RICHWIDGETIMAGE] = image;
  eventData[P_URL] = url_clean;
  SendEvent(E_RICHTEXT_IMAGE_REQUEST, eventData);
}

void RichImageProvider::CancelRequest(RichWidgetImage* image) {
  for (auto it = pending_requests_.Begin(); it != pending_requests_.End();) {
    if (it->second_ == image)
      it = pending_requests_.Erase(it);
    else
      ++it;
  }
}

void RichImageProvider::CompleteRequest(const String& url, Texture* texture) {
  auto it = pending_requests_.Find(StringHash(url));
  if (it != pending_requests_.End()) {
    if (it->second_) {
      it->second_->texture_ = texture;
      it->second_->GetParentWidget()->SetFlags(WidgetFlags_ContentChanged);
    }
    pending_requests_.Erase(it);
  }
}

} // namespace Urho3D