#include "NavMesh.h"
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <queue>
#include <algorithm>
#include "Core/Mesh.h"
#include "Core/Model.h"

// ---- Helpers ----

static glm::vec3 ComputeCentroid(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
	return (a + b + c) / 3.0f;
}

static glm::vec3 ComputeNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
	glm::vec3 n = glm::cross(b - a, c - a);
	float len = glm::length(n);
	return (len > 1e-8f) ? n / len : glm::vec3(0.0f, 1.0f, 0.0f);
}

static bool PointInTriangle3D(const glm::vec3& point,
	const glm::vec3& a, const glm::vec3& b, const glm::vec3& c,
	const glm::vec3& normal, float heightTolerance = 20.0f)
{
	float planeDist = glm::dot(normal, point - a);
	if (std::abs(planeDist) > heightTolerance)
		return false;

	glm::vec3 projected = point - normal * planeDist;

	glm::vec3 v0 = c - a;
	glm::vec3 v1 = b - a;
	glm::vec3 v2 = projected - a;

	float dot00 = glm::dot(v0, v0);
	float dot01 = glm::dot(v0, v1);
	float dot02 = glm::dot(v0, v2);
	float dot11 = glm::dot(v1, v1);
	float dot12 = glm::dot(v1, v2);

	float denom = dot00 * dot11 - dot01 * dot01;
	if (std::abs(denom) < 1e-10f)
		return false;

	float invDenom = 1.0f / denom;
	float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	return (u >= 0.0f) && (v >= 0.0f) && (u + v <= 1.0f);
}

static float Cross2D(const glm::vec3& o, const glm::vec3& a, const glm::vec3& b)
{
	return (a.x - o.x) * (b.z - o.z) - (a.z - o.z) * (b.x - o.x);
}

// ---- Build ----

int32_t NavMesh::AddTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
	NavTriangle tri;
	tri.vertices = { a, b, c };
	tri.centroid = ComputeCentroid(a, b, c);
	tri.normal = ComputeNormal(a, b, c);
	tri.neighbours = { -1, -1, -1 };
	tri.edgeDanger = 0.0f;

	int32_t index = static_cast<int32_t>(triangles.size());
	triangles.push_back(tri);
	return index;
}

void NavMesh::BuildFromModel(const std::shared_ptr<Model>& model,
	const glm::vec3& position,
	const glm::quat& rotation,
	const glm::vec3& scale,
	float maxSlopeAngleDeg)
{
	if (!model)
	{
		std::cout << "[NavMesh] BuildFromModel: null model, skipping." << std::endl;
		return;
	}

	const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
	const float maxSlopeCos = std::cos(glm::radians(maxSlopeAngleDeg));

	int32_t addedCount = 0;
	int32_t rejectedCount = 0;

	for (const Mesh& mesh : model->getMeshes())
	{
		const auto& vertices = mesh.getVertices();
		const auto& indices = mesh.getIndices();

		size_t triCount = indices.size() / 3;
		for (size_t i = 0; i < triCount; ++i)
		{
			glm::vec3 v0 = vertices[indices[i * 3 + 0]].position;
			glm::vec3 v1 = vertices[indices[i * 3 + 1]].position;
			glm::vec3 v2 = vertices[indices[i * 3 + 2]].position;

			v0 = position + rotation * (scale * v0);
			v1 = position + rotation * (scale * v1);
			v2 = position + rotation * (scale * v2);

			glm::vec3 normal = ComputeNormal(v0, v1, v2);

			float slopeDot = glm::dot(normal, worldUp);
			if (slopeDot < maxSlopeCos)
			{
				++rejectedCount;
				continue;
			}

			AddTriangle(v0, v1, v2);
			++addedCount;
		}
	}

	std::cout << "[NavMesh] BuildFromModel: " << addedCount << " walkable triangles added, "
		<< rejectedCount << " steep triangles rejected." << std::endl;
}

void NavMesh::Subdivide()
{
	size_t oldCount = triangles.size();
	std::vector<NavTriangle> newTriangles;
	newTriangles.reserve(oldCount * 4);

	for (size_t i = 0; i < oldCount; ++i)
	{
		const glm::vec3& a = triangles[i].vertices[0];
		const glm::vec3& b = triangles[i].vertices[1];
		const glm::vec3& c = triangles[i].vertices[2];

		glm::vec3 ab = (a + b) * 0.5f;
		glm::vec3 bc = (b + c) * 0.5f;
		glm::vec3 ac = (a + c) * 0.5f;

		NavTriangle t0, t1, t2, t3;

		t0.vertices = { a, ab, ac };
		t1.vertices = { ab, b, bc };
		t2.vertices = { ac, bc, c };
		t3.vertices = { ab, bc, ac };

		auto finish = [](NavTriangle& t) {
			t.centroid = (t.vertices[0] + t.vertices[1] + t.vertices[2]) / 3.0f;
			glm::vec3 n = glm::cross(t.vertices[1] - t.vertices[0], t.vertices[2] - t.vertices[0]);
			float len = glm::length(n);
			t.normal = (len > 1e-8f) ? n / len : glm::vec3(0.0f, 1.0f, 0.0f);
			t.neighbours = { -1, -1, -1 };
			t.edgeDanger = 0.0f;
			};

		finish(t0); finish(t1); finish(t2); finish(t3);

		newTriangles.push_back(t0);
		newTriangles.push_back(t1);
		newTriangles.push_back(t2);
		newTriangles.push_back(t3);
	}

	triangles = std::move(newTriangles);

	std::cout << "[NavMesh] Subdivided: " << oldCount << " -> " << triangles.size() << " triangles" << std::endl;
}

// ---- Adjacency ----

struct EdgeKey
{
	int64_t ax, ay, az, bx, by, bz;

	bool operator==(const EdgeKey& o) const
	{
		return ax == o.ax && ay == o.ay && az == o.az &&
			bx == o.bx && by == o.by && bz == o.bz;
	}
};

struct EdgeKeyHash
{
	size_t operator()(const EdgeKey& k) const
	{
		size_t h = 0;
		auto mix = [&](int64_t v) { h ^= std::hash<int64_t>()(v) + 0x9e3779b9 + (h << 6) + (h >> 2); };
		mix(k.ax); mix(k.ay); mix(k.az);
		mix(k.bx); mix(k.by); mix(k.bz);
		return h;
	}
};

static int64_t Quantise(float f)
{
	return static_cast<int64_t>(std::round(f * 1000.0));
}

static EdgeKey MakeEdgeKey(const glm::vec3& a, const glm::vec3& b)
{
	int64_t qa[3] = { Quantise(a.x), Quantise(a.y), Quantise(a.z) };
	int64_t qb[3] = { Quantise(b.x), Quantise(b.y), Quantise(b.z) };

	bool aFirst = std::tie(qa[0], qa[1], qa[2]) <= std::tie(qb[0], qb[1], qb[2]);
	if (aFirst)
		return { qa[0], qa[1], qa[2], qb[0], qb[1], qb[2] };
	else
		return { qb[0], qb[1], qb[2], qa[0], qa[1], qa[2] };
}

void NavMesh::BuildAdjacency()
{
	struct EdgeOwner
	{
		int32_t triIndex;
		int32_t edgeIndex;
	};

	std::unordered_map<EdgeKey, EdgeOwner, EdgeKeyHash> edgeMap;

	for (int32_t ti = 0; ti < static_cast<int32_t>(triangles.size()); ++ti)
	{
		auto& tri = triangles[ti];
		for (int e = 0; e < 3; ++e)
		{
			const glm::vec3& va = tri.vertices[e];
			const glm::vec3& vb = tri.vertices[(e + 1) % 3];

			EdgeKey key = MakeEdgeKey(va, vb);
			auto it = edgeMap.find(key);
			if (it != edgeMap.end())
			{
				int32_t otherTri = it->second.triIndex;
				int32_t otherEdge = it->second.edgeIndex;

				tri.neighbours[e] = otherTri;
				triangles[otherTri].neighbours[otherEdge] = ti;
			}
			else
			{
				edgeMap[key] = { ti, e };
			}
		}
	}

	int32_t adjacentPairs = 0;
	for (const auto& t : triangles)
		for (int e = 0; e < 3; ++e)
			if (t.neighbours[e] >= 0)
				++adjacentPairs;

	std::cout << "[NavMesh] Built adjacency: " << triangles.size()
		<< " triangles, " << (adjacentPairs / 2) << " shared edges" << std::endl;
}

// ---- Edge Danger ----

void NavMesh::ComputeEdgeDanger(int spreadRadius)
{
	int32_t count = static_cast<int32_t>(triangles.size());

	// Step 1: Mark boundary triangles (any edge with no neighbour = mesh boundary = potential cliff)
	int32_t boundaryCount = 0;
	for (int32_t i = 0; i < count; ++i)
	{
		for (int e = 0; e < 3; ++e)
		{
			if (triangles[i].neighbours[e] < 0)
			{
				triangles[i].edgeDanger = 1.0f;
				++boundaryCount;
				break;
			}
		}
	}

	// Step 2: BFS spread danger inward from boundary triangles
	// Each step reduces danger by 1/spreadRadius
	if (spreadRadius > 0)
	{
		std::queue<int32_t> frontier;
		std::vector<int> depth(count, -1);

		for (int32_t i = 0; i < count; ++i)
		{
			if (triangles[i].edgeDanger > 0.0f)
			{
				frontier.push(i);
				depth[i] = 0;
			}
		}

		while (!frontier.empty())
		{
			int32_t current = frontier.front();
			frontier.pop();

			int nextDepth = depth[current] + 1;
			if (nextDepth >= spreadRadius)
				continue;

			float nextDanger = 1.0f - (static_cast<float>(nextDepth) / static_cast<float>(spreadRadius));

			for (int e = 0; e < 3; ++e)
			{
				int32_t nb = triangles[current].neighbours[e];
				if (nb >= 0 && depth[nb] < 0)
				{
					depth[nb] = nextDepth;
					if (nextDanger > triangles[nb].edgeDanger)
						triangles[nb].edgeDanger = nextDanger;
					frontier.push(nb);
				}
			}
		}
	}

	std::cout << "[NavMesh] ComputeEdgeDanger: " << boundaryCount
		<< " boundary triangles, spread radius=" << spreadRadius << std::endl;
}

// ---- Queries ----

int32_t NavMesh::FindTriangle(const glm::vec3& point) const
{
	for (int32_t i = 0; i < static_cast<int32_t>(triangles.size()); ++i)
	{
		const auto& tri = triangles[i];
		if (PointInTriangle3D(point, tri.vertices[0], tri.vertices[1], tri.vertices[2], tri.normal))
			return i;
	}
	return -1;
}

int32_t NavMesh::FindClosestTriangle(const glm::vec3& point) const
{
	if (triangles.empty())
		return -1;

	int32_t bestIndex = -1;
	float bestDistSq = std::numeric_limits<float>::max();

	for (int32_t i = 0; i < static_cast<int32_t>(triangles.size()); ++i)
	{
		float distSq = glm::dot(triangles[i].centroid - point, triangles[i].centroid - point);
		if (distSq < bestDistSq)
		{
			bestDistSq = distSq;
			bestIndex = i;
		}
	}
	return bestIndex;
}

// ---- A* Pathfinding ----

NavPath NavMesh::FindPath(int32_t startTri, int32_t goalTri,
	const glm::vec3& startPos, const glm::vec3& goalPos,
	float edgePenaltyWeight) const
{
	NavPath result;

	if (startTri < 0 || goalTri < 0 ||
		startTri >= static_cast<int32_t>(triangles.size()) ||
		goalTri >= static_cast<int32_t>(triangles.size()))
	{
		return result;
	}

	if (startTri == goalTri)
	{
		result.triangleIndices.push_back(startTri);
		result.waypoints.push_back(startPos);
		result.waypoints.push_back(goalPos);
		return result;
	}

	using Node = std::pair<float, int32_t>;
	std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;

	std::vector<float> gCost(triangles.size(), std::numeric_limits<float>::max());
	std::vector<int32_t> cameFrom(triangles.size(), -1);
	std::vector<bool> closed(triangles.size(), false);

	gCost[startTri] = 0.0f;
	float h = glm::length(triangles[startTri].centroid - triangles[goalTri].centroid);
	openSet.push({ h, startTri });

	bool found = false;

	while (!openSet.empty())
	{
		auto [fCost, current] = openSet.top();
		openSet.pop();

		if (current == goalTri)
		{
			found = true;
			break;
		}

		if (closed[current])
			continue;
		closed[current] = true;

		const auto& tri = triangles[current];
		for (int e = 0; e < 3; ++e)
		{
			int32_t neighbour = tri.neighbours[e];
			if (neighbour < 0 || closed[neighbour])
				continue;

			float edgeCost = glm::length(triangles[neighbour].centroid - tri.centroid);

			// Penalize triangles near edges/cliffs -- makes A* prefer interior paths
			float dangerPenalty = triangles[neighbour].edgeDanger * edgePenaltyWeight;
			float tentativeG = gCost[current] + edgeCost + dangerPenalty;

			if (tentativeG < gCost[neighbour])
			{
				gCost[neighbour] = tentativeG;
				cameFrom[neighbour] = current;

				float hCost = glm::length(triangles[neighbour].centroid - triangles[goalTri].centroid);
				openSet.push({ tentativeG + hCost, neighbour });
			}
		}
	}

	if (!found)
		return result;

	// Reconstruct corridor
	std::vector<int32_t> corridor;
	for (int32_t cur = goalTri; cur != -1; cur = cameFrom[cur])
		corridor.push_back(cur);
	std::reverse(corridor.begin(), corridor.end());

	result.triangleIndices = corridor;

	// Build waypoints from corridor centroids
	result.waypoints.push_back(startPos);

	const int sampleInterval = 3;
	for (size_t i = 1; i < corridor.size(); ++i)
	{
		if (i % sampleInterval == 0 || i == corridor.size() - 1)
			result.waypoints.push_back(triangles[corridor[i]].centroid);
	}

	if (glm::length(result.waypoints.back() - goalPos) > 0.01f)
		result.waypoints.push_back(goalPos);

	// Step 1: Cull waypoints that are too close together
	{
		const float minSpacing = 5.0f;
		std::vector<glm::vec3> culled;
		culled.push_back(result.waypoints.front());

		for (size_t i = 1; i < result.waypoints.size() - 1; ++i)
		{
			float distFromLast = glm::length(result.waypoints[i] - culled.back());
			if (distFromLast >= minSpacing)
			{
				culled.push_back(result.waypoints[i]);
			}
		}

		culled.push_back(result.waypoints.back());
		result.waypoints = culled;
	}

	// Step 2: Generate a smooth Catmull-Rom spline through the waypoints.
	// This replaces sharp corners with natural arcs -- the spline inherently
	// creates wide turns because it matches the tangent at each control point.
	if (result.waypoints.size() > 2)
	{
		const int subdivisions = 4; // points to insert between each pair of waypoints

		std::vector<glm::vec3> spline;
		size_t n = result.waypoints.size();

		for (size_t i = 0; i < n - 1; ++i)
		{
			// Catmull-Rom needs 4 control points: p0, p1, p2, p3
			// Clamp at the ends
			glm::vec3 p0 = result.waypoints[(i > 0) ? i - 1 : 0];
			glm::vec3 p1 = result.waypoints[i];
			glm::vec3 p2 = result.waypoints[i + 1];
			glm::vec3 p3 = result.waypoints[(i + 2 < n) ? i + 2 : n - 1];

			// Always include the control point itself
			spline.push_back(p1);

			// Insert subdivided points between p1 and p2
			for (int s = 1; s < subdivisions; ++s)
			{
				float t = static_cast<float>(s) / static_cast<float>(subdivisions);
				float t2 = t * t;
				float t3 = t2 * t;

				// Catmull-Rom basis
				glm::vec3 point =
					0.5f * ((2.0f * p1) +
					(-p0 + p2) * t +
					(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
					(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);

				spline.push_back(point);
			}
		}

		// Add the final point
		spline.push_back(result.waypoints.back());

		result.waypoints = spline;
	}

	// Step 3: Cull the spline output to a reasonable density for the AI to follow
	{
		const float finalSpacing = 5.0f;
		std::vector<glm::vec3> culled;
		culled.push_back(result.waypoints.front());

		for (size_t i = 1; i < result.waypoints.size() - 1; ++i)
		{
			float distFromLast = glm::length(result.waypoints[i] - culled.back());
			if (distFromLast >= finalSpacing)
			{
				culled.push_back(result.waypoints[i]);
			}
		}

		culled.push_back(result.waypoints.back());
		result.waypoints = culled;
	}

	return result;
}