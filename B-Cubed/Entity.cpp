#include "Entity.h"
#include "DirectXMath.h"
#include "Gui.h"



void Entity::AddRenderable(std::unique_ptr<Renderable> pRenderable)
{
	renderable = std::move(pRenderable);
}

void Entity::Render(Graphics & gfx, const DirectX::XMMATRIX& camera, const Light& light)
{
	struct VertexConstant
	{
		DirectX::XMMATRIX transform;
		DirectX::XMMATRIX perspective;
		DirectX::XMMATRIX rollpitchyaw;
	};

	struct PixelConstant
	{
		DirectX::XMFLOAT4 light;
	};
	
	VertexConstant vc
	{
		DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll)*
		DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z)*
		camera,
		DirectX::XMMatrixPerspectiveLH(1.0f, float(gfx.GetHeight()) / float(gfx.GetWidth()), 0.5f, 100.0f),
		DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll)
	};

	PixelConstant pc
	{
		light.GetPosition()
	};


	renderable->UpdateVertex(gfx, vc);
	renderable->UpdatePixel(gfx, pc);

	renderable->Render(gfx);
}

void Entity::SetPosition(float x, float y, float z)
{
	pos = DirectX::XMFLOAT3(x,y,z);
}
