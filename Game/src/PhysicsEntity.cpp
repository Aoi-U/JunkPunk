#include "PhysicsEntity.h"

Frustum CreateFrustum(float zFar, float zNear, float fovY, float aspectRatio, glm::vec3 front, glm::vec3 right, glm::vec3 up, glm::vec3 pos) {
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

AABB generateAABB(Model& model)
{
	glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

	for (const Mesh& mesh : model.getMeshes())
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

	for (const Mesh& mesh : model->getMeshes())
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

PhysicsEntity::PhysicsEntity(const std::string name, DrawType dt, PhysicsType pt, const std::string path)
	: BaseEntity(name, dt, pt), model(std::make_unique<Model>(path))
{
	bv = std::make_unique<AABB>(generateAABB(*model));
}

void PhysicsEntity::updateSelfAndChild(float deltaTime)
{
	if (transform.getIsDirty())
	{
		forceUpdateSelfAndChild(deltaTime);
		return;
	}

	for (auto&& child : children)
	{
		child->updateSelfAndChild(deltaTime);
	}
}

void PhysicsEntity::forceUpdateSelfAndChild(float deltaTime)
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
		child->forceUpdateSelfAndChild(deltaTime);
	}
}

//void PhysicsEntity::drawSelfAndChild(const Frustum& frust, const std::shared_ptr<Renderer> rend, const std::shared_ptr<Shader> shader, bool isShadowPass, int& numDrawed)
//{
//	if (isShadowPass)
//	{
//		shader->setMat4("u_model", transform.getModelMatrix());
//		rend->DrawModelShadow(model.get());
//	}
//	else if (bv->isOnFrustum(frust, transform))
//	{
//		shader->setMat4("u_model", transform.getModelMatrix());
//		rend->DrawModel(model.get(), shader);
//		numDrawed++;
//	}
//}

