#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <list>
#include <array>

#include "Model.h"
#include "Camera.h"
#include "Shader.h"
#include "Renderer.h"

class Transform
{
public:
	void computeModelMatrix()
	{
		modelMatrix = getLocalModelMatrix();
		isDirty = false;
	}

	void computeModelMatrix(const glm::mat4& parentGlobalModelMatrix)
	{
		modelMatrix = parentGlobalModelMatrix * getLocalModelMatrix();
		isDirty = false;
	}

	void setLocalPosition(const glm::vec3& newPosition)
	{
		pos = newPosition;
		isDirty = true;
	}

	void setLocalRotation(const glm::quat& newRotation)
	{
		quatRot = newRotation;
		isDirty = true;
	}

	void setLocalScale(const glm::vec3& newScale)
	{
		scale = newScale;
		isDirty = true;
	}

	const glm::vec3& getGlobalPosition() const { return modelMatrix[3]; }
	const glm::vec3 getLocalPosition() const { return pos; }
	const glm::quat& getLocalRotation() const { return quatRot; }
	const glm::vec3& getLocalScale() const { return scale; }

	const glm::mat4& getModelMatrix() const { return modelMatrix; }

	glm::vec3 getRight() const { return modelMatrix[0]; }
	glm::vec3 getUp() const { return modelMatrix[1]; }
	glm::vec3 getForward() const { return -modelMatrix[2]; }
	glm::vec3 getBack() const { return modelMatrix[2]; }

	glm::vec3 getGlobalScale() const { return { glm::length(getRight()), glm::length(getUp()), glm::length(getForward()) }; }

	bool getIsDirty() const { return isDirty; }

protected:
	glm::vec3 pos = glm::vec3(0.0f);
	glm::quat quatRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	glm::mat4 modelMatrix = glm::mat4(1.0f);

	bool isDirty = true;

	glm::mat4 getLocalModelMatrix()
	{
		glm::mat4 rotationMatrix = glm::mat4_cast(quatRot);
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), pos);
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
		return translationMatrix * rotationMatrix * scaleMatrix;
	}
};

struct Plane
{
	glm::vec3 normal = { 0.0f, 1.0f, 0.0f };
	float distance = 0.0f;

	Plane() = default;
	Plane(const glm::vec3& point, const glm::vec3& norm)
		: normal(glm::normalize(norm)), distance(glm::dot(norm, point))
	{
	}

	float getSignedDistanceToPoint(const glm::vec3& point) const
	{
		return glm::dot(normal, point) - distance;
	}
};

struct Frustum
{
	Plane top;
	Plane bottom;
	Plane left;
	Plane right;
	Plane near;
	Plane far;
};

// abstract base class that represents a bounding volume
struct BoundingVolume
{
	virtual bool isOnFrustum(const Frustum& frust, const Transform& transform) const = 0;

	virtual bool isOnOrForwardPlane(const Plane& plane) const = 0;

	bool isOnFrustum(const Frustum& frust) const
	{
		return(
			isOnOrForwardPlane(frust.left) &&
			isOnOrForwardPlane(frust.right) &&
			isOnOrForwardPlane(frust.top) &&
			isOnOrForwardPlane(frust.bottom) &&
			isOnOrForwardPlane(frust.far) &&
			isOnOrForwardPlane(frust.near)
			);
	}
};

struct Sphere : public BoundingVolume
{
	Sphere(const glm::vec3& inCenter, float inRadius)
		: BoundingVolume(), center(inCenter), radius(inRadius) 
	{
	}

	bool isOnOrForwardPlane(const Plane& plane) const final
	{
		return plane.getSignedDistanceToPoint(center) > -radius;
	}

	bool isOnFrustum(const Frustum& frust, const Transform& transform) const final
	{
		const glm::vec3 globalScale = transform.getGlobalScale();
		const glm::vec3 globalCenter = transform.getModelMatrix() * glm::vec4(center, 1.0f);
		const float maxScale = glm::max(glm::max(globalScale.x, globalScale.y), globalScale.z);

		Sphere globalSphere(globalCenter, radius * (maxScale * 0.5f));

		return (
			globalSphere.isOnOrForwardPlane(frust.left) &&
			globalSphere.isOnOrForwardPlane(frust.right) &&
			globalSphere.isOnOrForwardPlane(frust.top) &&
			globalSphere.isOnOrForwardPlane(frust.bottom) &&
			globalSphere.isOnOrForwardPlane(frust.far) &&
			globalSphere.isOnOrForwardPlane(frust.near) 
			);
	}

	glm::vec3 center{ 0.0f, 0.0f, 0.0f };
	float radius{ 0.0f };
};

struct AABB : public BoundingVolume
{
	AABB(const glm::vec3& min, const glm::vec3& max)
		: BoundingVolume(), center((max + min) * 0.5f), extents(max.x - center.x, max.y - center.y, max.z - center.z)
	{
	}

	AABB(const glm::vec3& inCenter, float inExtentsX, float inExtentsY, float inExtentsZ)
		: BoundingVolume(), center(inCenter), extents(inExtentsX, inExtentsY, inExtentsZ)
	{
	}
	
	std::array<glm::vec3, 8> getVertices() const
	{
		std::array<glm::vec3, 8> vertices;
		vertices[0] = { center.x - extents.x, center.y - extents.y, center.z - extents.z };
		vertices[1] = { center.x + extents.x, center.y - extents.y, center.z - extents.z };
		vertices[2] = { center.x - extents.x, center.y + extents.y, center.z - extents.z };
		vertices[3] = { center.x + extents.x, center.y + extents.y, center.z - extents.z };
		vertices[4] = { center.x - extents.x, center.y - extents.y, center.z + extents.z };
		vertices[5] = { center.x + extents.x, center.y - extents.y, center.z + extents.z };
		vertices[6] = { center.x - extents.x, center.y + extents.y, center.z + extents.z };
		vertices[7] = { center.x + extents.x, center.y + extents.y, center.z + extents.z };
		return vertices;
	}

	bool isOnOrForwardPlane(const Plane& plane) const final
	{
		const float r =
			extents.x * glm::abs(plane.normal.x) +
			extents.y * glm::abs(plane.normal.y) +
			extents.z * glm::abs(plane.normal.z);
	
		return -r <= plane.getSignedDistanceToPoint(center);
	}

	bool isOnFrustum(const Frustum& frust, const Transform& transform) const final
	{
		const glm::vec3 globalCenter = transform.getModelMatrix() * glm::vec4(center, 1.0f);

		// scaled orientation
		const glm::vec3 right = transform.getRight() * extents.x;
		const glm::vec3 up = transform.getUp() * extents.y;
		const glm::vec3 forward = transform.getForward() * extents.z;

		const float newExtentsX =
			glm::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), right)) +
			glm::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), up)) +
			glm::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), forward));
		const float newExtentsY =
			glm::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), right)) +
			glm::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), up)) +
			glm::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), forward));
		const float newExtentsZ =
			glm::abs(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), right)) +
			glm::abs(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), up)) +
			glm::abs(glm::dot(glm::vec3(0.0f, 0.0, 1.0f), forward));

		const AABB globalAABB(globalCenter, newExtentsX, newExtentsY, newExtentsZ);

		return (
			globalAABB.isOnOrForwardPlane(frust.left) &&
			globalAABB.isOnOrForwardPlane(frust.right) &&
			globalAABB.isOnOrForwardPlane(frust.top) &&
			globalAABB.isOnOrForwardPlane(frust.bottom) &&
			globalAABB.isOnOrForwardPlane(frust.far) &&
			globalAABB.isOnOrForwardPlane(frust.near)
			);
	}


	glm::vec3 center{ 0.0f, 0.0f, 0.0f };
	glm::vec3 extents{ 0.0f, 0.0f, 0.0f };
};

Frustum CreateFrustum(const std::shared_ptr<Camera> camera);

AABB generateAABB(std::shared_ptr<Model> model);
Sphere generateBoundingSphere(std::shared_ptr<Model> model);

// entity class to represent objects in the game 
class Entity 
{
public:
	Entity(const std::string path);
	
	void Cleanup();

	template<typename... TArgs>
	void addChild(const TArgs&... args);
	
	void updateSelfAndChild();

	void forceUpdateSelfAndChild();

	void drawSelfAndChild(const Frustum& frust, std::shared_ptr<Renderer> renderer, std::shared_ptr<Shader>& shader, bool isShadowpass, int& numDrawed);

	std::vector<Mesh>& getMeshes() { return model->getMeshes(); }

	std::shared_ptr<Model> getModel() { return model; }

	Transform transform;

	std::vector<std::unique_ptr<Entity>> children; // scene graph
	Entity* parent = nullptr;

private:
	std::shared_ptr<Model> model;
	std::unique_ptr<AABB> boundingVolume;
};
