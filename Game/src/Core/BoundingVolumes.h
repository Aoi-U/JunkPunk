#pragma once

#include <memory>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform;
class Model;

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
	Plane nearPlane;
	Plane farPlane;
};

// abstract base class that represents a bounding volume
struct BoundingVolume
{
	virtual bool isOnFrustum(const Frustum& frust, const glm::mat4 modelMatrix) const = 0;

	virtual bool isOnOrForwardPlane(const Plane& plane) const = 0;

	bool isOnFrustum(const Frustum& frust) const
	{
		return(
			isOnOrForwardPlane(frust.left) &&
			isOnOrForwardPlane(frust.right) &&
			isOnOrForwardPlane(frust.top) &&
			isOnOrForwardPlane(frust.bottom) &&
			isOnOrForwardPlane(frust.farPlane) &&
			isOnOrForwardPlane(frust.nearPlane)
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

	bool isOnFrustum(const Frustum& frust, const glm::mat4 modelMatrix) const final
	{
		glm::vec3 scale = {
			glm::length(glm::vec3(modelMatrix[0])),
			glm::length(glm::vec3(modelMatrix[1])),
			glm::length(glm::vec3(modelMatrix[2]))
		};


		glm::vec3 globalScale = scale;
		glm::vec3 globalCenter = modelMatrix * glm::vec4(center, 1.0f);
		float maxScale = (std::max)(globalScale.x, globalScale.y);
		maxScale = (std::max)(maxScale, globalScale.z);

		Sphere globalSphere(globalCenter, radius * (maxScale * 0.5f));

		return (
			globalSphere.isOnOrForwardPlane(frust.left) &&
			globalSphere.isOnOrForwardPlane(frust.right) &&
			globalSphere.isOnOrForwardPlane(frust.top) &&
			globalSphere.isOnOrForwardPlane(frust.bottom) &&
			globalSphere.isOnOrForwardPlane(frust.farPlane) &&
			globalSphere.isOnOrForwardPlane(frust.nearPlane)
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
			extents.x * std::abs(plane.normal.x) +
			extents.y * std::abs(plane.normal.y) +
			extents.z * std::abs(plane.normal.z);

		return -r <= plane.getSignedDistanceToPoint(center);
	}

	bool isOnFrustum(const Frustum& frust, const glm::mat4 modelMatrix) const final
	{
		const glm::vec3 globalCenter = modelMatrix * glm::vec4(center, 1.0f);

		// scaled orientation
		const glm::vec3 right = glm::vec3(modelMatrix[0]) * extents.x;
		const glm::vec3 up = glm::vec3(modelMatrix[1]) * extents.y;
		const glm::vec3 forward = glm::vec3(modelMatrix[2]) * extents.z;

		const float newExtentsX =
			std::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), right)) +
			std::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), up)) +
			std::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), forward));
		const float newExtentsY =
			std::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), right)) +
			std::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), up)) +
			std::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), forward));
		const float newExtentsZ =
			std::abs(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), right)) +
			std::abs(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), up)) +
			std::abs(glm::dot(glm::vec3(0.0f, 0.0, 1.0f), forward));

		const AABB globalAABB(globalCenter, newExtentsX, newExtentsY, newExtentsZ);

		return (
			globalAABB.isOnOrForwardPlane(frust.left) &&
			globalAABB.isOnOrForwardPlane(frust.right) &&
			globalAABB.isOnOrForwardPlane(frust.top) &&
			globalAABB.isOnOrForwardPlane(frust.bottom) &&
			globalAABB.isOnOrForwardPlane(frust.farPlane) &&
			globalAABB.isOnOrForwardPlane(frust.nearPlane)
			);
	}

	glm::vec3 center{ 0.0f, 0.0f, 0.0f };
	glm::vec3 extents{ 0.0f, 0.0f, 0.0f };
};

Frustum CreateFrustum(float zFar, float zNear, float fovY, float aspectRatio, glm::vec3 front, glm::vec3 right, glm::vec3 up, glm::vec3 pos); // creates the camera frustum

AABB generateAABB(const Model& model); // generates an aabb for a model
Sphere generateBoundingSphere(std::shared_ptr<Model> model); // generates a bounding sphere for a model