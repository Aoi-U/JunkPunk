#include "Entity.h"

Frustum CreateFrustum(const std::shared_ptr<Camera> camera) {
	float zFar = camera->GetFarClipPlane();
	float zNear = camera->GetNearClipPlane();
	float fovY = glm::radians(camera->GetFov());
	float aspectRatio = camera->GetAspectRatio();
	glm::vec3 front = camera->GetFrontVector();
	glm::vec3 right = camera->GetRightVector();
	glm::vec3 up = camera->GetUpVector();
	glm::vec3 pos = camera->GetPosition();


	Frustum frust;
	const float halfVSide = zFar * tanf(fovY * 0.5f);
	const float halfHSide = halfVSide * aspectRatio;
	const glm::vec3 frontMultFar = zFar * front;

	frust.near = { pos + zNear * front, front };
	frust.far = { pos + frontMultFar, -front };
	frust.right = { pos, glm::normalize(glm::cross(frontMultFar - right * halfHSide, up)) };
	frust.left = { pos, glm::normalize(glm::cross(up, frontMultFar + right * halfHSide)) };
	frust.top = { pos, glm::normalize(glm::cross(right, frontMultFar - up * halfVSide)) };
	frust.bottom = { pos, glm::normalize(glm::cross(frontMultFar + up * halfVSide, right)) };

	return frust;
}

AABB generateAABB(std::shared_ptr<Model> model)
{
	glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

	for (Mesh& mesh : model->getMeshes())
	{
		for (const Vertex& vertex : mesh.getVertices())
		{
			minAABB.x = glm::min(minAABB.x, vertex.position.x);
			minAABB.y = glm::min(minAABB.y, vertex.position.y);
			minAABB.z = glm::min(minAABB.z, vertex.position.z);
									
			maxAABB.x = glm::max(maxAABB.x, vertex.position.x);
			maxAABB.y = glm::max(maxAABB.y, vertex.position.y);
			maxAABB.z = glm::max(maxAABB.z, vertex.position.z);
		}
	}
	return AABB(minAABB, maxAABB);
}

Sphere generateBoundingSphere(std::shared_ptr<Model> model)
{
	glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

	for (Mesh& mesh : model->getMeshes())
	{
		for (const Vertex& vertex : mesh.getVertices())
		{
			minAABB.x = glm::min(minAABB.x, vertex.position.x);
			minAABB.y = glm::min(minAABB.y, vertex.position.y);
			minAABB.z = glm::min(minAABB.z, vertex.position.z);
																						 
			maxAABB.x = glm::max(maxAABB.x, vertex.position.x);
			maxAABB.y = glm::max(maxAABB.y, vertex.position.y);
			maxAABB.z = glm::max(maxAABB.z, vertex.position.z);
		}
	}

	return Sphere((maxAABB + minAABB) * 0.5f, glm::length(minAABB - maxAABB));
}

Entity::Entity(const std::string name, const std::string path)
	: name(name), model(std::make_shared<Model>(path))
{
	boundingVolume = std::make_unique<AABB>(generateAABB(model));
}

void Entity::updateSelfAndChild()
{
	if (transform.getIsDirty())
	{
		forceUpdateSelfAndChild();
		return;
	}

	for (auto&& child : children)
	{
		child->updateSelfAndChild();
	}
}

void Entity:: forceUpdateSelfAndChild()
{
	if (parent)
	{
		transform.computeModelMatrix(parent->transform.getModelMatrix());
	}
	else
	{
		transform.computeModelMatrix();
	}

	for (auto&& child : children)
	{
		child->forceUpdateSelfAndChild();
	}
}

Entity* Entity::getChild(std::string name)
{
	for (const std::unique_ptr<Entity>& child : children)
	{
		if (child->name == name)
		{
			return child.get();
		}
	}

	std::cout << "Child " << name << " not found" << std::endl;
	return nullptr;
}

void Entity::drawSelfAndChild(const Frustum& frust, const std::shared_ptr<Renderer> renderer, const std::shared_ptr<Shader> shader, bool isShadowpass, int& numDrawed)
{
	if (isShadowpass) // ignore frustum for shadow pass
	{
		shader->setMat4("u_model", transform.getModelMatrix());
		renderer->DrawModelShadow(model);
	}
	else if (boundingVolume->isOnFrustum(frust, transform)) // frustum culling
	{
		numDrawed++;
		shader->setMat4("u_model", transform.getModelMatrix());
		renderer->DrawModel(model, shader);
	}

	for (auto&& child : children)
	{
		child->drawSelfAndChild(frust, renderer, shader, isShadowpass, numDrawed);
	}
}
