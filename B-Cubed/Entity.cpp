#include "Entity.h"
#include "DirectXMath.h"
#include "Gui.h"

void Entity::AddRenderable(std::unique_ptr<Renderable> pRenderable)
{
	renderable = std::move(pRenderable);
}

void Entity::Render(Graphics & gfx)
{
	Gui::AddSlider("pitch", pitch, -2 * 3.14f, 2 * 3.14f);
	Gui::AddSlider("yaw", yaw, -2 * 3.14f, 2 * 3.14f);
	Gui::AddSlider("roll", roll, -2 * 3.14f, 2 * 3.14f);
	Gui::AddSlider("distance", distance, 0.0f, 40.0f);

	// temp
	struct Transform
	{
		DirectX::XMMATRIX transform;
	};
	Transform transform =
	{
		DirectX::XMMatrixRotationRollPitchYaw(pitch,yaw,roll)*
		DirectX::XMMatrixTranslation(0.0f,0.0f, distance)*
		DirectX::XMMatrixPerspectiveLH(1.0f, float(gfx.GetHeight()) / float(gfx.GetWidth()),1.0f,40.0f)
	};
	// ----
	renderable->Update(gfx, transform);
	renderable->Render(gfx);
}