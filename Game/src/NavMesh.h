#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <array>
#include <cstdint>
#include <limits>
#include <memory>

class Model;

struct NavTriangle
{
	std::array<glm::vec3, 3> vertices;
	glm::vec3 centroid = glm::vec3(0.0f);
	glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
	std::array<int32_t, 3> neighbours = { -1, -1, -1 };

	// How close this triangle is to an edge/cliff. 0 = safe interior, 1.0 = right on the edge.
	// Used by A* to penalize paths near drop-offs.
	float edgeDanger = 0.0f;
};

struct NavPath
{
	std::vector<int32_t> triangleIndices;
	std::vector<glm::vec3> waypoints;
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
		float maxSlopeAngleDeg = 40.0f);

	void Subdivide();
	void BuildAdjacency();

	// Bridge disconnected navmesh islands by linking closest triangles across gaps.
	// maxGapDistance = maximum horizontal (XZ) distance to bridge between islands.
	// maxHeightDiff = maximum vertical (Y) difference to allow bridging (prevents connecting walls/cliffs).
	void StitchDisconnectedIslands(float maxGapDistance, float maxHeightDiff = 15.0f);

	// Count the number of disconnected components in the navmesh graph.
	// Returns the number of islands (1 = fully connected).
	int32_t CountConnectedComponents() const;

	// Call after BuildAdjacency. Computes edgeDanger for each triangle and
	// propagates it inward so A* avoids paths near cliffs.
	// spreadRadius = how many triangles inward the danger spreads (default 3).
	void ComputeEdgeDanger(int spreadRadius = 3);

	// ---- Queries ----
	int32_t FindTriangle(const glm::vec3& point) const;
	int32_t FindClosestTriangle(const glm::vec3& point) const;

	// Like FindClosestTriangle but only considers triangles within maxHeightDiff
	// of the query point's Y. Prevents snapping to walls/cliffs above or below.
	int32_t FindClosestTriangleAtHeight(const glm::vec3& point, float maxHeightDiff = 5.0f) const;

	// Like FindTriangle but only matches if the triangle is within maxHeightDiff of the point's Y
	int32_t FindTriangleAtHeight(const glm::vec3& point, float maxHeightDiff = 5.0f) const;

	// edgePenaltyWeight: how much to penalize edge-adjacent triangles (0 = no penalty, higher = more avoidance)
	NavPath FindPath(int32_t startTri, int32_t goalTri,
		const glm::vec3& startPos, const glm::vec3& goalPos,
		float edgePenaltyWeight = 5.0f) const;

	// ---- Accessors ----
	const std::vector<NavTriangle>& GetTriangles() const { return triangles; }
	size_t TriangleCount() const { return triangles.size(); }
	bool IsEmpty() const { return triangles.empty(); }

private:
	std::vector<NavTriangle> triangles;
};