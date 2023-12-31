set(PELLET_LIBRARY_NAME pellet)
set(CMAKE_CXX_STANDARD 17)

add_library(${PELLET_LIBRARY_NAME} STATIC
    Bullet3Collision/BroadPhaseCollision/b3DynamicBvh.cpp
    Bullet3Collision/BroadPhaseCollision/b3DynamicBvhBroadphase.cpp
    Bullet3Collision/BroadPhaseCollision/b3OverlappingPairCache.cpp
    Bullet3Collision/NarrowPhaseCollision/b3ConvexUtility.cpp
    Bullet3Collision/NarrowPhaseCollision/b3CpuNarrowPhase.cpp
    Bullet3Common/b3AlignedAllocator.cpp
    Bullet3Common/b3Logging.cpp
    Bullet3Common/b3Vector3.cpp
    Bullet3Dynamics/b3CpuRigidBodyPipeline.cpp
    Bullet3Dynamics/ConstraintSolver/b3FixedConstraint.cpp
    Bullet3Dynamics/ConstraintSolver/b3Generic6DofConstraint.cpp
    Bullet3Dynamics/ConstraintSolver/b3PgsJacobiSolver.cpp
    Bullet3Dynamics/ConstraintSolver/b3Point2PointConstraint.cpp
    Bullet3Dynamics/ConstraintSolver/b3TypedConstraint.cpp
    Bullet3Geometry/b3ConvexHullComputer.cpp
    Bullet3Geometry/b3GeometryUtil.cpp
    BulletCollision/BroadphaseCollision/btAxisSweep3.cpp
    BulletCollision/BroadphaseCollision/btBroadphaseProxy.cpp
    BulletCollision/BroadphaseCollision/btCollisionAlgorithm.cpp
    BulletCollision/BroadphaseCollision/btDbvt.cpp
    BulletCollision/BroadphaseCollision/btDbvtBroadphase.cpp
    BulletCollision/BroadphaseCollision/btDispatcher.cpp
    BulletCollision/BroadphaseCollision/btOverlappingPairCache.cpp
    BulletCollision/BroadphaseCollision/btQuantizedBvh.cpp
    BulletCollision/BroadphaseCollision/btSimpleBroadphase.cpp
    BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.cpp
    BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.cpp
    BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.cpp
    BulletCollision/CollisionDispatch/btBoxBoxDetector.cpp
    BulletCollision/CollisionDispatch/btCollisionDispatcher.cpp
    BulletCollision/CollisionDispatch/btCollisionObject.cpp
    BulletCollision/CollisionDispatch/btCollisionWorld.cpp
    BulletCollision/CollisionDispatch/btCollisionWorldImporter.cpp
    BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.cpp
    BulletCollision/CollisionDispatch/btCompoundCompoundCollisionAlgorithm.cpp
    BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.cpp
    BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.cpp
    BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.cpp
    BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.cpp
    BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.cpp
    BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.cpp
    BulletCollision/CollisionDispatch/btGhostObject.cpp
    BulletCollision/CollisionDispatch/btHashedSimplePairCache.cpp
    BulletCollision/CollisionDispatch/btInternalEdgeUtility.cpp
    BulletCollision/CollisionDispatch/btManifoldResult.cpp
    BulletCollision/CollisionDispatch/btSimulationIslandManager.cpp
    BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.cpp
    BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.cpp
    BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.cpp
    BulletCollision/CollisionDispatch/btUnionFind.cpp
    BulletCollision/CollisionDispatch/SphereTriangleDetector.cpp
    BulletCollision/CollisionShapes/btBox2dShape.cpp
    BulletCollision/CollisionShapes/btBoxShape.cpp
    BulletCollision/CollisionShapes/btBvhTriangleMeshShape.cpp
    BulletCollision/CollisionShapes/btCapsuleShape.cpp
    BulletCollision/CollisionShapes/btCollisionShape.cpp
    BulletCollision/CollisionShapes/btCompoundShape.cpp
    BulletCollision/CollisionShapes/btConcaveShape.cpp
    BulletCollision/CollisionShapes/btConeShape.cpp
    BulletCollision/CollisionShapes/btConvex2dShape.cpp
    BulletCollision/CollisionShapes/btConvexHullShape.cpp
    BulletCollision/CollisionShapes/btConvexInternalShape.cpp
    BulletCollision/CollisionShapes/btConvexPointCloudShape.cpp
    BulletCollision/CollisionShapes/btConvexPolyhedron.cpp
    BulletCollision/CollisionShapes/btConvexShape.cpp
    BulletCollision/CollisionShapes/btConvexTriangleMeshShape.cpp
    BulletCollision/CollisionShapes/btCylinderShape.cpp
    BulletCollision/CollisionShapes/btEmptyShape.cpp
    BulletCollision/CollisionShapes/btHeightfieldTerrainShape.cpp
    BulletCollision/CollisionShapes/btMinkowskiSumShape.cpp
    BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.cpp
    BulletCollision/CollisionShapes/btMultiSphereShape.cpp
    BulletCollision/CollisionShapes/btOptimizedBvh.cpp
    BulletCollision/CollisionShapes/btPolyhedralConvexShape.cpp
    BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.cpp
    BulletCollision/CollisionShapes/btShapeHull.cpp
    BulletCollision/CollisionShapes/btSphereShape.cpp
    BulletCollision/CollisionShapes/btStaticPlaneShape.cpp
    BulletCollision/CollisionShapes/btStridingMeshInterface.cpp
    BulletCollision/CollisionShapes/btTetrahedronShape.cpp
    BulletCollision/CollisionShapes/btTriangleBuffer.cpp
    BulletCollision/CollisionShapes/btTriangleCallback.cpp
    BulletCollision/CollisionShapes/btTriangleIndexVertexArray.cpp
    BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.cpp
    BulletCollision/CollisionShapes/btTriangleMesh.cpp
    BulletCollision/CollisionShapes/btTriangleMeshShape.cpp
    BulletCollision/CollisionShapes/btUniformScalingShape.cpp
    BulletCollision/Gimpact/btContactProcessing.cpp
    BulletCollision/Gimpact/btGenericPoolAllocator.cpp
    BulletCollision/Gimpact/btGImpactBvh.cpp
    BulletCollision/Gimpact/btGImpactCollisionAlgorithm.cpp
    BulletCollision/Gimpact/btGImpactQuantizedBvh.cpp
    BulletCollision/Gimpact/btGImpactShape.cpp
    BulletCollision/Gimpact/btTriangleShapeEx.cpp
    BulletCollision/Gimpact/gim_box_set.cpp
    BulletCollision/Gimpact/gim_contact.cpp
    BulletCollision/Gimpact/gim_memory.cpp
    BulletCollision/Gimpact/gim_tri_collision.cpp
    BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.cpp
    BulletCollision/NarrowPhaseCollision/btConvexCast.cpp
    BulletCollision/NarrowPhaseCollision/btGjkConvexCast.cpp
    BulletCollision/NarrowPhaseCollision/btGjkEpa2.cpp
    BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.cpp
    BulletCollision/NarrowPhaseCollision/btGjkPairDetector.cpp
    BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.cpp
    BulletCollision/NarrowPhaseCollision/btPersistentManifold.cpp
    BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.cpp
    BulletCollision/NarrowPhaseCollision/btRaycastCallback.cpp
    BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.cpp
    BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.cpp
    BulletDynamics/Character/btKinematicCharacterController.cpp
    BulletDynamics/ConstraintSolver/btConeTwistConstraint.cpp
    BulletDynamics/ConstraintSolver/btContactConstraint.cpp
    BulletDynamics/ConstraintSolver/btFixedConstraint.cpp
    BulletDynamics/ConstraintSolver/btGearConstraint.cpp
    BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.cpp
    BulletDynamics/ConstraintSolver/btGeneric6DofSpring2Constraint.cpp
    BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.cpp
    BulletDynamics/ConstraintSolver/btHinge2Constraint.cpp
    BulletDynamics/ConstraintSolver/btHingeConstraint.cpp
    BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.cpp
    BulletDynamics/ConstraintSolver/btPoint2PointConstraint.cpp
    BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp
    BulletDynamics/ConstraintSolver/btSliderConstraint.cpp
    BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.cpp
    BulletDynamics/ConstraintSolver/btTypedConstraint.cpp
    BulletDynamics/ConstraintSolver/btUniversalConstraint.cpp
    BulletDynamics/Dynamics/btDiscreteDynamicsWorld.cpp
    BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.cpp
    BulletDynamics/Dynamics/btRigidBody.cpp
    BulletDynamics/Dynamics/btSimpleDynamicsWorld.cpp
    BulletDynamics/Dynamics/btSimulationIslandManagerMt.cpp
    BulletDynamics/Featherstone/btMultiBody.cpp
    BulletDynamics/Featherstone/btMultiBodyConstraint.cpp
    BulletDynamics/Featherstone/btMultiBodyConstraintSolver.cpp
    BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.cpp
    BulletDynamics/Featherstone/btMultiBodyFixedConstraint.cpp
    BulletDynamics/Featherstone/btMultiBodyJointLimitConstraint.cpp
    BulletDynamics/Featherstone/btMultiBodyJointMotor.cpp
    BulletDynamics/Featherstone/btMultiBodyPoint2Point.cpp
    BulletDynamics/Featherstone/btMultiBodySliderConstraint.cpp
    BulletDynamics/MLCPSolvers/btDantzigLCP.cpp
    BulletDynamics/MLCPSolvers/btLemkeAlgorithm.cpp
    BulletDynamics/MLCPSolvers/btMLCPSolver.cpp
    BulletDynamics/Vehicle/btRaycastVehicle.cpp
    BulletDynamics/Vehicle/btWheelInfo.cpp
    BulletInverseDynamics/details/MultiBodyTreeImpl.cpp
    BulletInverseDynamics/details/MultiBodyTreeInitCache.cpp
    BulletInverseDynamics/IDMath.cpp
    BulletInverseDynamics/MultiBodyTree.cpp
    BulletSoftBody/btDefaultSoftBodySolver.cpp
    BulletSoftBody/btSoftBody.cpp
    BulletSoftBody/btSoftBodyConcaveCollisionAlgorithm.cpp
    BulletSoftBody/btSoftBodyHelpers.cpp
    BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.cpp
    BulletSoftBody/btSoftMultiBodyDynamicsWorld.cpp
    BulletSoftBody/btSoftRigidCollisionAlgorithm.cpp
    BulletSoftBody/btSoftRigidDynamicsWorld.cpp
    BulletSoftBody/btSoftSoftCollisionAlgorithm.cpp
    LinearMath/btAlignedAllocator.cpp
    LinearMath/btConvexHull.cpp
    LinearMath/btConvexHullComputer.cpp
    LinearMath/btGeometryUtil.cpp
    LinearMath/btPolarDecomposition.cpp
    LinearMath/btQuickprof.cpp
    LinearMath/btSerializer.cpp
    LinearMath/btThreads.cpp
    LinearMath/btVector3.cpp
)

target_include_directories(${PELLET_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

