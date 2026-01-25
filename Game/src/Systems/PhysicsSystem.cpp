#include "PhysicsSystem.h"
#include "../Components/Transform.h"
#include "../Components/Physics.h"
#include "../Components/Player.h"

#include "../ECSController.h"

extern ECSController controller;

PhysicsSystem::PhysicsSystem()
	: gVehicle()
{
	controller.AddEventListener(Events::Player::PLAYER_JUMPED, [this](Event& e) { this->PhysicsSystem::JumpEventListener(e); });
	controller.AddEventListener(Events::Player::RESET_VEHICLE, [this](Event& e) { this->PhysicsSystem::ResetVehicleEventListener(e); });

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
	//PhysicsSceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	PhysicsSceneDesc.gravity = PxVec3(0.0f, -20.0f, 0.0f); // double gravity for vehicle testing
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	PhysicsSceneDesc.cpuDispatcher = gDispatcher;
	PhysicsSceneDesc.filterShader = PxDefaultSimulationFilterShader;


	//PhysicsSceneDesc.filterShader = VehicleFilterShader;
	gPhysicsScene = gPhysics->createScene(PhysicsSceneDesc);

	gPhysicsScene->setSimulationEventCallback(gPhysicsCallbacks);

	// https://nvidia-omniverse.github.io/PhysX/physx/5.6.1/docs/DebugVisualization.html
	gPhysicsScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	gPhysicsScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
	gPhysicsScene->setVisualizationParameter(PxVisualizationParameter::eBODY_AXES, 1.0f);
	
	// vehicle specific visualizations

	gGroundMaterial = gPhysics->createMaterial(1.0f, 1.0f, 1.0f);


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

void PhysicsSystem::Init()
{
	Box(1.0f, 5, PxVec3(0.0f, 10.0f, 0.0f)); // test
	CreateMap();
}

void PhysicsSystem::Update(float deltaTime)
{
	// get the vehicle command from ECS
	Entity vehicleEntity = controller.GetEntityByTag("VehicleCommands");
	auto& vehicleCommands = controller.GetComponent<VehicleCommands>(vehicleEntity);

	vehicleCommands.isGrounded = gVehicle.IsGrounded(gPhysicsScene);

	Command command;
	command.throttle = vehicleCommands.throttle;
	command.brake = vehicleCommands.brake;
	command.steer = vehicleCommands.steer;
	gVehicle.setCommand(command);

	gVehicle.step(deltaTime);
	gPhysicsScene->simulate(deltaTime);
	gPhysicsScene->fetchResults(true);

	// Update dynamic actor transforms in ECS
	for (auto& entity : entities)
	{
		auto& transform = controller.GetComponent<Transform>(entity);
		
		if (controller.HasComponent<RigidBody>(entity))
		{
			auto& rigidBody = controller.GetComponent<RigidBody>(entity);

			PxTransform pxTransform = rigidBody.actor->getGlobalPose();
			transform.position = glm::vec3(pxTransform.p.x, pxTransform.p.y, pxTransform.p.z);
			transform.quatRotation = glm::quat(pxTransform.q.w, pxTransform.q.x, pxTransform.q.y, pxTransform.q.z);
		}
		else if (controller.HasComponent<VehicleBody>(entity))
		{
			auto& vehicleBody = controller.GetComponent<VehicleBody>(entity);
			PxTransform pxTransform = gVehicle.getTransform();
			transform.position = glm::vec3(pxTransform.p.x, pxTransform.p.y, pxTransform.p.z);
			transform.quatRotation = glm::quat(pxTransform.q.w, pxTransform.q.x, pxTransform.q.y, pxTransform.q.z);

			PxVec3 linearVel = gVehicle.getLinearVelocity();
			PxVec3 angularVel = gVehicle.getAngularVelocity();
			vehicleBody.linearVelocity = glm::vec3(linearVel.x, linearVel.y, linearVel.z);
			vehicleBody.angularVelocity = glm::vec3(angularVel.x, angularVel.y, angularVel.z);
		}
	}
}

void PhysicsSystem::Plane()
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

void PhysicsSystem::Box(float halfLen, PxU32 size, PxVec3 position)
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


			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			gPhysicsScene->addActor(*body);
		}
	}
	shape->release();
}


void PhysicsSystem::Simulate(float deltaTime)
{
	gVehicle.step(deltaTime);
	gPhysicsScene->simulate(deltaTime);
	gPhysicsScene->fetchResults(true);
}

void PhysicsSystem::Cleanup()
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

void PhysicsSystem::CreateMap()
{
	// get all entities with StaticModel and Transform components
	for (auto& entity : entities)
	{
		auto& transform = controller.GetComponent<Transform>(entity);

		if (controller.HasComponent<StaticModel>(entity))
		{
			auto& entityModel = controller.GetComponent<StaticModel>(entity);
			for (const auto& mesh : entityModel.collisionMesh->getMeshes())
			{
				PxTriangleMesh* triangleMesh = CreateTriangleMesh(mesh);
				if (!triangleMesh)
				{
					std::cout << "Failed to create triangle mesh for entity!" << std::endl;
					continue;
				}
				
				PxMeshScale scale = PxMeshScale(PxVec3(transform.scale.x, transform.scale.y, transform.scale.z), PxQuat(PxIdentity)); 
				PxTransform meshTransform = PxTransform(PxVec3(transform.position.x, transform.position.y, transform.position.z),
					PxQuat(transform.quatRotation.x, transform.quatRotation.y, transform.quatRotation.z, transform.quatRotation.w));
				
				PxShape* shape = gPhysics->createShape(PxTriangleMeshGeometry(triangleMesh, scale), *gGroundMaterial);
				shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
				shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
				shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);

				PxRigidStatic* staticActor = gPhysics->createRigidStatic(meshTransform);
				staticActor->attachShape(*shape);
				staticActor->setActorFlag(PxActorFlag::eVISUALIZATION, false);

				auto& actor = controller.GetComponent<StaticModel>(entity);
				actor.actor = staticActor;

				gPhysicsScene->addActor(*staticActor);
				shape->release();
				triangleMesh->release();
			}
		}
		else if (controller.HasComponent<RigidBody>(entity))
		{
			// create dynamic rigid body
		}
		else if (controller.HasComponent<VehicleBody>(entity))
		{
			if (!gVehicle.setup(gPhysicsScene, gPhysics, gGroundMaterial))
			{
				std::cout << "Vehicle failed to initialize!" << std::endl;
				return;
			}
		}
		else if (controller.HasComponent<Trigger>(entity))
		{
			// create trigger volume
		}

	}
}

PxTriangleMesh* PhysicsSystem::CreateTriangleMesh(const Mesh& mesh)
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

	params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES);
	params.meshWeldTolerance = 0.001f;

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

void PhysicsSystem::JumpEventListener(Event& e)
{
	gVehicle.jump();
}

void PhysicsSystem::ResetVehicleEventListener(Event& e)
{
	//gVehicle.respawnAtCheckpoint();
	gVehicle.resetTransform();
}
