#ifndef __RICH_IMAGE_PROVIDER_H__
#define __RICH_IMAGE_PROVIDER_H__
#pragma once

#include <Urho3D/Core/Object.h>
#include "rich_batch_image.h"

namespace Urho3D {

/// RichTextImageRequest
URHO3D_EVENT(E_RICHTEXT_IMAGE_REQUEST, RichTextImageRequest) {
  URHO3D_PARAM(P_RICHWIDGETIMAGE, Image);      // RichWidgetImage
  URHO3D_PARAM(P_URL, Url);      // image url
}

// Rich widgets use this subsystem class to query for images
// When an image is ready, call CompleteRequest
class RichImageProvider: public Object {
  URHO3D_OBJECT(RichImageProvider, Object);
public:
  RichImageProvider(Context* context);
  ~RichImageProvider() override;

  // This method will fire an event of type E_RICHIMAGE_REQUEST
  void RequestImageResource(RichWidgetImage* image, const String& url);

  void CancelRequest(RichWidgetImage* image);

  // When image is ready (or not available), this method must be called.
  // If no image is available, texture can be set to 0
  void CompleteRequest(const String& url, Texture* texture);
private:
  Urho3D::HashMap<StringHash, WeakPtr<RichWidgetImage>> pending_requests_;
};

} // namespace Urho3D

#endif
