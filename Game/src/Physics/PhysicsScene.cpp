#include "PhysicsScene.h"

PhysicsScene::PhysicsScene()
	: gVehicle()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	if (!gFoundation)
	{
		std::cout << "PxCreateFoundation failed!" << std::endl;
		return;
	}

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);


	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
	if (!gPhysics)
	{
		std::cout << "PxCreatePhysics failed!" << std::endl;
		return;
	}

	PxSceneDesc PhysicsSceneDesc(gPhysics->getTolerancesScale());
	PhysicsSceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	PhysicsSceneDesc.cpuDispatcher = gDispatcher;
	PhysicsSceneDesc.filterShader = PxDefaultSimulationFilterShader;
	//PhysicsSceneDesc.filterShader = VehicleFilterShader;
	gPhysicsScene = gPhysics->createScene(PhysicsSceneDesc);

	// https://nvidia-omniverse.github.io/PhysX/physx/5.6.1/docs/DebugVisualization.html
	gPhysicsScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	gPhysicsScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
	gPhysicsScene->setVisualizationParameter(PxVisualizationParameter::eBODY_AXES, 1.0f);
	
	// vehicle specific visualizations

	gGroundMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);


	// initialize PVD
	
	PxPvdSceneClient* pvdClient = gPhysicsScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	PxInitVehicleExtension(*gFoundation); // initialize vehicle extension
}

void PhysicsScene::Plane()
{
	gGroundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gGroundMaterial);
	for (PxU32 i = 0; i < gGroundPlane->getNbShapes(); i++)
	{
		PxShape* shape = NULL;
		gGroundPlane->getShapes(&shape, 1, i);
		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
	}

	gPhysicsScene->addActor(*gGroundPlane);
}

void PhysicsScene::InitPhysicsComponentFromEntity(BaseEntity* entity)
{
	if (!entity)
	{
		std::cout << "Null entity passed to InitPhysicsComponentFromEntity" << std::endl;
		return;
	}

	PhysicsEntity* physicsEntity = dynamic_cast<PhysicsEntity*>(entity);
	
	if (physicsEntity)
	{
		std::cout << "Initializing physics component for entity: " << physicsEntity->name << " with physics type: " << static_cast<int>(physicsEntity->physicsType) << std::endl;
		switch (physicsEntity->physicsType)
		{
		case PhysicsType::STATIC_MESH:
			CreateStaticPhysicsComponent(physicsEntity);
			break;
		case PhysicsType::VEHICLE:
			std::cout << "Creating vehicle physics component for entity: " << physicsEntity->name << std::endl;
			CreateVehiclePhysicsComponent(physicsEntity);
			break;
		case PhysicsType::RIGID_BODY:
			CreateDynamicPhysicsComponent(physicsEntity);
			break;
		case PhysicsType::KINEMATIC:
			CreateKinematicPhysicsComponent(physicsEntity);
			break;
		case PhysicsType::TRIGGER:
			CreateTriggerPhysicsComponent(physicsEntity);
			break;
		default:
			break;
		}
	}
	
	for (auto& child : entity->children)
	{
		InitPhysicsComponentFromEntity(child.get());
	}
}

void PhysicsScene::Box(float halfLen, PxU32 size, PxVec3 position)
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfLen, halfLen, halfLen), *gGroundMaterial);
	PxTransform tran(position);

	// Create a pyramid of physics-enabled boxes
	for (PxU32 i = 0; i < size; i++)
	{
		for (PxU32 j = 0; j < size - i; j++)
		{
			PxTransform localTran(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 - 1), 0 * halfLen));
			PxRigidDynamic* body = gPhysics->createRigidDynamic(tran.transform(localTran));

			dynamicActors.insert({ std::to_string(i), body });

			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			gPhysicsScene->addActor(*body);
		}
	}
	shape->release();
}

void PhysicsScene::InitPhysics()
{
	Box(1.0f, 5, PxVec3(0.0f, 10.0f, 0.0f)); // test
	//Map(entities);
	if (!gVehicle.setup(gPhysicsScene, gPhysics, gGroundMaterial))
	{
		std::cout << "Vehicle failed to initialize!" << std::endl;
		return;
	}
}

void PhysicsScene::Simulate(float deltaTime)
{
	gVehicle.step(deltaTime);
	gPhysicsScene->simulate(deltaTime);
	gPhysicsScene->fetchResults(true);
}

void PhysicsScene::Cleanup()
{
	PxCloseVehicleExtension();
	gPhysicsScene->release();
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

PxVec3 PhysicsScene::GetDynamicActorPos(std::string name)
{
	return dynamicActors[name]->getGlobalPose().p;
}


void PhysicsScene::CreateStaticPhysicsComponent(const PhysicsEntity* entity)
{
	std::cout << "Creating physics entity for: " << entity->name << std::endl;
	for (const Mesh mesh : entity->getModel()->getMeshes())
	{
		PxTriangleMesh* triangleMesh = CreateTriangleMesh(mesh);
		if (!triangleMesh)
		{
			std::cout << "Failed to create triangle mesh for entity: " << entity->name << std::endl;
			continue;
		}

		glm::vec3 scale = entity->transform.getGlobalScale();
		PxMeshScale meshScale = PxMeshScale(PxVec3(scale.x, scale.y, scale.z), PxQuat(PxIdentity));

		PxShape* shape;
		shape = gPhysics->createShape(PxTriangleMeshGeometry(triangleMesh, meshScale), *gGroundMaterial);
			
		// set collision flags
		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);

		glm::vec3 pos = entity->transform.getGlobalPosition();
		PxTransform tran = PxTransform(PxVec3(pos.x, pos.y, pos.z));

		PxRigidStatic* staticBody = gPhysics->createRigidStatic(tran);
		staticBody->attachShape(*shape);
		staticBody->setActorFlag(PxActorFlag::eVISUALIZATION, false);
		gPhysicsScene->addActor(*staticBody);

		shape->release();
		triangleMesh->release();
	}
}

void PhysicsScene::CreateVehiclePhysicsComponent(const PhysicsEntity* entity)
{
	std::cout << "called" << std::endl;
	if (!gVehicle.setup(gPhysicsScene, gPhysics, gGroundMaterial))
	{
		std::cout << "Vehicle failed to initialize for entity: " << entity->name << std::endl;
		return;
	}
}

void PhysicsScene::CreateDynamicPhysicsComponent(const PhysicsEntity* entity)
{

}

void PhysicsScene::CreateKinematicPhysicsComponent(const PhysicsEntity* entity)
{

}

void PhysicsScene::CreateTriggerPhysicsComponent(const PhysicsEntity* entity)
{

}

PxTriangleMesh* PhysicsScene::CreateTriangleMesh(const Mesh& mesh)
{
	std::vector<PxVec3> positions;
	positions.reserve(mesh.getVertices().size());
	for (const Vertex& v : mesh.getVertices())
	{
		positions.emplace_back(PxVec3(v.position.x, v.position.y, v.position.z));
	}

	// https://nvidia-omniverse.github.io/PhysX/physx/5.6.1/docs/Geometry.html
	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = static_cast<PxU32>(positions.size());
	meshDesc.points.stride = sizeof(PxVec3);
	meshDesc.points.data = positions.data();

	meshDesc.triangles.count = static_cast<PxU32>(mesh.getIndices().size() / 3);
	meshDesc.triangles.stride = 3 * sizeof(GLuint);
	meshDesc.triangles.data = mesh.getIndices().data();

	

	PxTolerancesScale scale = PxTolerancesScale();
	PxCookingParams params(scale);

	//params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES);
	//params.meshWeldTolerance = 0.001f;

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