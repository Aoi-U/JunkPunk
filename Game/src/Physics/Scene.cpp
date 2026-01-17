#include "Scene.h"

Scene::Scene()
	: gVehicle()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	if (!gFoundation)
	{
		std::cout << "PxCreateFoundation failed!" << std::endl;
		return;
	}

}

void Scene::InitPVD()
{
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	PxInitVehicleExtension(*gFoundation);
}

void Scene::InitScene()
{
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
	if (!gPhysics)
	{
		std::cout << "PxCreatePhysics failed!" << std::endl;
		return;
	}

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	//sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	sceneDesc.filterShader = VehicleFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	// https://nvidia-omniverse.github.io/PhysX/physx/5.6.1/docs/DebugVisualization.html
	gScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	gScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
	//gScene->setVisualizationParameter(PxVisualizationParameter::eBODY_AXES, 1.0f);
	gScene->setVisualizationParameter(PxVisualizationParameter::eBODY_MASS_AXES, 1.0f);
	// vehicle specific visualizations

	// ---------------------------------------------------------------------------------

	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
}

void Scene::PrepPVD()
{
	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
}

void Scene::Plane()
{
	gGroundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
	for (PxU32 i = 0; i < gGroundPlane->getNbShapes(); i++)
	{
		PxShape* shape = NULL;
		gGroundPlane->getShapes(&shape, 1, i);
		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
	}

	gScene->addActor(*gGroundPlane);
}

void Scene::Map(std::vector<Entity>& entities)
{
	for (size_t i = 0; i < entities.size(); i++)
	{
		for (size_t j = 0; j < entities[i].getModel()->getMeshes().size(); j++)
		{
			Mesh& mesh = entities[i].getModel()->getMeshes()[j];
			PxTriangleMesh* triangleMesh = CreateTriangleMesh(mesh);
			if (!triangleMesh)
			{
				std::cout << "Failed to create triangle mesh for entity " + i << std::endl;
				return;
			}
			PxShape* shape = gPhysics->createShape(PxTriangleMeshGeometry(triangleMesh), *gMaterial);
			PxTransform tran(PxVec3(0.0f, 0.0f, 0.0f));
			glm::mat4 modelMatrix = entities[i].getModelMatrix();

			// set the mesh position
			glm::vec3 position = modelMatrix[3];
			tran.p = PxVec3(position.x, position.y, position.z);

			// set the mesh scale
			glm::vec3 scale = glm::vec3(modelMatrix[0][0], modelMatrix[1][1], modelMatrix[2][2]);
			PxMeshScale meshScale(PxVec3(scale.x, scale.y, scale.z), PxQuat(PxIdentity));
			shape->setGeometry(PxTriangleMeshGeometry(triangleMesh, meshScale));

			PxRigidStatic* staticBody = gPhysics->createRigidStatic(tran);
			staticBody->attachShape(*shape);
			staticBody->setActorFlag(PxActorFlag::eVISUALIZATION, false);
			gScene->addActor(*staticBody);
			shape->release();
			triangleMesh->release();
		}
	}
}

void Scene::Box(float halfLen, PxU32 size, PxVec3 position)
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfLen, halfLen, halfLen), *gMaterial);
	PxTransform tran(position);

	// Create a pyramid of physics-enabled boxes
	for (PxU32 i = 0; i < size; i++)
	{
		for (PxU32 j = 0; j < size - i; j++)
		{
			PxTransform localTran(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 - 1), 0 * halfLen));
			PxRigidDynamic* body = gPhysics->createRigidDynamic(tran.transform(localTran));
			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			gScene->addActor(*body);
		}
	}
	shape->release();
}

void Scene::InitPhysics(std::vector<Entity>& entities)
{
	InitPVD();
	InitScene();
	PrepPVD();
	//Plane();
	Map(entities);
	gVehicle.initMaterialFrictionTable(gMaterial);
	if (!gVehicle.initVehicles(gScene, gPhysics, gMaterial))
	{
		std::cout << "Failed to initialize vehicles!" << std::endl;
		return;
	}
}

void Scene::Simulate(float deltaTime)
{
	gVehicle.step(deltaTime);
	gScene->simulate(1 / 60.0f);
	gScene->fetchResults(true);
}

void Scene::Cleanup()
{
	PxCloseVehicleExtension();
	gScene->release();
	gDispatcher->release();
	gPhysics->release();
	if (gPvd)
	{
		PxPvdTransport* transport = gPvd->getTransport();
		gPvd->release();
		transport->release();
	}
	gFoundation->release();
}

const PxRenderBuffer& Scene::GetRenderBuffer()
{
	return gScene->getRenderBuffer();
}



PxTriangleMesh* Scene::CreateTriangleMesh(Mesh& mesh)
{
	// https://nvidia-omniverse.github.io/PhysX/physx/5.6.1/docs/Geometry.html
	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = static_cast<PxU32>(mesh.getVertices().size());
	meshDesc.points.stride = sizeof(Vertex);
	meshDesc.points.data = mesh.getVertices().data();

	meshDesc.triangles.count = static_cast<PxU32>(mesh.getIndices().size() / 3);
	meshDesc.triangles.stride = 3 * sizeof(GLuint);
	meshDesc.triangles.data = mesh.getIndices().data();

	PxTolerancesScale scale;
	PxCookingParams params(scale);

	PxDefaultMemoryOutputStream writeBuffer;
	PxTriangleMeshCookingResult::Enum result;
	bool status = PxCookTriangleMesh(params, meshDesc, writeBuffer, &result);
	if (!status)
	{
		std::cout << "Failed to cook triangle mesh" << std::endl;
		return NULL;
	}
	PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

	return gPhysics->createTriangleMesh(readBuffer);
}
