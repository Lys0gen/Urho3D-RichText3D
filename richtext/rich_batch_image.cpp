#include "rich_batch_image.h"
#include "rich_widget.h"
#include "rich_image_provider.h"
#include "Urho3D/Graphics/Drawable.h"
#include "Urho3D/Core/Context.h"

namespace Urho3D {

/// Register object factory. Drawable must be registered first.
void RichWidgetImage::RegisterObject(Context* context)
{
    context->RegisterFactory<RichWidgetImage>();
}

RichWidgetImage::RichWidgetImage(Context* context) 
 : RichWidgetBatch(context)
{
     ResourceCache* cache = GetSubsystem<ResourceCache>();
     Material* material = cache->GetResource<Material>("Materials/RichImage.xml");
     material_ = material->Clone();
     material_->SetTexture(TU_DIFFUSE, 0);
     material_->SetTexture(TU_NORMAL, 0);
     material_->SetTexture(TU_SPECULAR, 0);
     material_->SetName("RichWidgetImage");
}


RichWidgetImage::~RichWidgetImage()
{
  auto image_provider = context_->GetSubsystem<RichImageProvider>();
  if (image_provider)
    image_provider->CancelRequest(this);
}


void RichWidgetImage::SetImageSource(const String& sourceUrl)
{
    this->source_url_ = sourceUrl;
    if (!texture_/* || sourceUrl != texture_->GetName()*/)
    {
        if (context_->GetSubsystem<ResourceCache>()->Exists(sourceUrl))
            texture_ = context_->GetSubsystem<ResourceCache>()->GetResource<Texture2D>(sourceUrl);
        else {
          // request image from the RichImageProvider subsystem
          auto image_provider = context_->GetSubsystem<RichImageProvider>();
          image_provider->RequestImageResource(this, sourceUrl);
        }
        if (texture_ && material_)
        {
            material_->SetTexture(TU_DIFFUSE, texture_);
            if (parent_widget_)
                parent_widget_->SetFlags(WidgetFlags_ContentChanged);
        }
    }
}

void RichWidgetImage::AddImage(const Vector3 pos, float width, float height)
{
    if (!texture_)
        return;

    if (parent_widget_ && parent_widget_->GetShadowEnabled())
    {
        AddQuad(
          Rect(pos.x_ + parent_widget_->GetShadowOffset().x_, 
              pos.y_ + parent_widget_->GetShadowOffset().y_, 
              pos.x_ + parent_widget_->GetShadowOffset().x_ + width,
              pos.y_ + parent_widget_->GetShadowOffset().y_ + height), 
            pos.z_ + 0.01f,
            Rect(0, 0, 0, 0), // NOTE: UV is empty
            parent_widget_->GetShadowColor());
    }
    AddQuad(Rect(pos.x_, pos.y_, pos.x_ + width, pos.y_ + height), pos.z_, Rect(0.0f, 0.0f, 1.0f, 1.0f), Color::WHITE);
}

int RichWidgetImage::GetImageWidth() const
{
    if (texture_)
        return texture_->GetWidth();
    return 0;
}

int RichWidgetImage::GetImageHeight() const
{
    if (texture_)
        return texture_->GetHeight();
    return 0;
}

float RichWidgetImage::GetImageAspect() const
{
    if (texture_ && texture_->GetHeight())
        return (float)texture_->GetWidth() / texture_->GetHeight();
    return 0;
}

} // namespace Urho3D
