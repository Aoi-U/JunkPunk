#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <array>
#include <cstdint>
#include <limits>
#include <memory>

class Model;

// A single triangle on the navigation mesh
struct NavTriangle
{
	std::array<glm::vec3, 3> vertices;          // The 3 corner positions
	glm::vec3 centroid = glm::vec3(0.0f);       // Precomputed centre
	glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);

	// Indices into NavMesh::triangles for up to 3 edge-adjacent neighbours.
	// -1 means no neighbour on that edge.
	std::array<int32_t, 3> neighbours = { -1, -1, -1 };
};

// Lightweight handle used during A* / path queries
struct NavPath
{
	std::vector<int32_t> triangleIndices;        // Corridor of triangle IDs
	std::vector<glm::vec3> waypoints;            // Smoothed world-space points
};

class NavMesh
{
public:
	// ---- Build ----
	int32_t AddTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

	void BuildFromModel(const std::shared_ptr<Model>& model,
		const glm::vec3& position,
		const glm::quat& rotation,
		const glm::vec3& scale,
		float maxSlopeAngleDeg = 60.0f);

	// Subdivide all triangles: each triangle is split into 4 by connecting edge midpoints.
	// Call this between BuildFromModel and BuildAdjacency to increase density.
	// Can be called multiple times (each call quadruples the triangle count).
	void Subdivide();

	void BuildAdjacency();

	// ---- Queries ----
	int32_t FindTriangle(const glm::vec3& point) const;
	int32_t FindClosestTriangle(const glm::vec3& point) const;

	NavPath FindPath(int32_t startTri, int32_t goalTri, const glm::vec3& startPos, const glm::vec3& goalPos) const;

	// ---- Accessors ----
	const std::vector<NavTriangle>& GetTriangles() const { return triangles; }
	size_t TriangleCount() const { return triangles.size(); }
	bool IsEmpty() const { return triangles.empty(); }

private:
	std::vector<NavTriangle> triangles;
};