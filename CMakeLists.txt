
cmake_minimum_required(VERSION 3.15)
#add_compile_options(/showIncludes)

project(fpsProjectServer)
set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Bt")
add_compile_options(/showIncludes)

if(MSVC)
    add_definitions(-D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
    add_definitions(-D_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(/std:c++latest)
if(MSVC)
    add_compile_options(/utf-8 /FS)
endif()
add_executable(fpsProjectServer
        main.cpp
        Session/GameSession.cpp
        util/util.cpp
        http-client/DedicateServerNotifier.cpp
        server-status/ServerStat.cpp
        http-listener/httpRestClient.cpp
        Session/netcode/SessionNetworkDto.cpp
        Session/sessionPool/SessionManager.cpp
        Session/sessionPool/SessionWorkerPool.cpp
        Session/netcode/CreateSessionNetworkDto.cpp
        Socket/EnetClient.cpp
        Socket/dto/AssignDto.cpp
        Socket/dto/MoveDto.cpp
        Socket/dto/DefaultDto.cpp
        Session/Game/Player.cpp
        Session/Game/PhysicsSystem.cpp
        Session/Game/MapManager.cpp
        Session/FhishiX/TagManager.h
        Session/FhishiX/triangle.h
        Session/FhishiX/gameobject/GameObjectArgument.h
        Session/Game/KDTree.h
        Session/FhishiX/AABB.h
        Session/FhishiX/FhishiX.h
        Session/Game/KDTree.cpp
        Socket/dto/BroadcastMoveDto.h
        Session/Game/GameLogic/Gun.h
        Session/Game/GameLogic/InteractableObject.h
        Session/FhishiX/Layer.h
        Session/FhishiX/gameobject/Collider/Collider.h
        Session/Game/Property.cpp
        Session/Game/Property.h
        Session/SessionUtil.cpp
        Session/SessionUtil.h
        Session/Game/data/staticData/weapon.h
        Session/FhishiX/gameobject/ColliderType.h
        Session/FhishiX/gameobject/Collider/BoxCollider.h
        Session/FhishiX/gameobject/Transform.h
        Session/FhishiX/gameobject/Collider/MeshCollider.h
        Session/FhishiX/quaternion/Quaternion.h
        Session/FhishiX/Matrix/Matrix.h
        Session/FhishiX/Matrix/Matrix3.h
        Session/FhishiX/Matrix/Matrix4.h
        Session/FhishiX/quaternion/Quaternion.cpp
        Session/FhishiX/gameobject/Collider/CapsuleCollider.h
        Session/FhishiX/gameobject/GameObjectArgument.cpp
        Session/SessionDXViewer/DirectXCore.cpp
        Session/SessionDXViewer/WIN32PROC.h
        Session/SessionDXViewer/DirectXCore.h
        Session/FhishiX/vector/Vector2Int.h
        ServerStatics.h
        Session/Component/Definition/ComponentArgument.h
        Session/FhishiX/gameobject/GameObjectManager.h
        Session/FhishiX/gameobject/GameObject.cpp
        Session/Component/Definition/ComponentManager.cpp
        Session/Component/Definition/ComponentManager.h
        Session/Component/Definition/ComponentHandle.h
        Session/FhishiX/gameobject/EntityTypes.h
        Session/Component/Definition/ComponentHandle.cpp
        Session/FhishiX/gameobject/rigidBody/RigidBody.cpp
        Session/FhishiX/gameobject/rigidBody/RigidBody.h
        Session/Component/Definition/ComponentArgument.cpp
        Session/FhishiX/gameobject/GameObjectManager.cpp
        Session/Component/Definition/ComponentHandleBase.h
        Session/SessionContext.h
        Session/SessionContext.h
        Session/Component/Definition/ComponentArgument.cpp
        Session/Component/Definition/ComponentArgument.h
        Session/Component/Definition/ComponentTypeCounter.h
        Session/Component/Definition/ComponentTypeCounter.h
        Session/FhishiX/gameobject/collider/CapsuleCollider.cpp
        Session/FhishiX/gameobject/collider/MeshCollider.cpp
        Session/FhishiX/gameobject/collider/Collider.cpp
        Session/FhishiX/gameobject/collider/BoxCollider.cpp
        Session/Component/Definition/ComponentFactory.h
        Session/FhishiX/gameobject/collider/StaticCollider.h
        Session/Game/PhysicsSystem.h
        Session/Dto/MapInfo.cpp
        Session/Game/Map/MapConstructer/ObjectConstructor.cpp
        Session/Game/Map/MapConstructer/ObjectConstructor.h
        Session/Game/Map/MapConstructer/ComponentConstructor.cpp
        Session/Game/Map/MapConstructer/ComponentConstructor.h
        Session/Component/Definition/Component.cpp
        Session/Component/Definition/Component.h
        Session/Component/Definition/ComponentFactory.cpp
        Session/Game/Map/MapConstructer/PhysicsSystemConstructor.h
        Session/FhishiX/gameobject/collider/StaticCollider.cpp
        InfoLoader/InfoLoader.cpp
        InfoLoader/InfoLoader.h
        Session/Dto/MapInfoArgument.h
        Session/SessionDXViewer/DebugViewStatic.h
        Session/SessionDXViewer/DebugViewStatic.h
        Session/SessionDXViewer/Camera.h
        ServerStatics.cpp
        Session/FhishiX/Renderer.h
        Session/FhishiX/Mesh/MeshManager.h
        Session/FhishiX/Mesh/Mesh.h
        Session/FhishiX/gameobject/collider/SphereCollider.cpp
        Session/FhishiX/gameobject/collider/SphereCollider.h
        Session/FhishiX/Mesh/StaticMeshMaker.cpp
        Session/Component/Implementation/DebugLogger.h
        Session/Component/Implementation/DebugLogger.cpp
        Session/Time.cpp
        Session/Time.h
        Session/FhishiX/PhysicsManager.h
        Session/FhishiX/PhysicsManager.cpp
        util/Log.h
        PCH.h
        Session/FhishiX/gameobject/collider/CollisionSolver.h
        Session/FhishiX/gameobject/collider/CollisionSover.cpp
        Session/FhishiX/gameobject/collider/Raycast.h
        Session/FhishiX/gameobject/collider/ColliderMaterial.h
        util/StringUtil.h
        MonitorUtil.h
        PrefabSystem/PrefabManager.cpp
        PrefabSystem/PrefabManager.h
        objectPool.h
        Session/Component/Implementation/Weapon.cpp
        Session/Component/Implementation/Weapon.h
        Session/Component/Implementation/CharacterSummonPosition.cpp
        Session/Component/Implementation/CharacterSummonPosition.h
        Session/Component/Implementation/GamePlayManager.cpp
        Session/Component/Implementation/GamePlayManager.h
        Session/InGamePhase.h
        Session/Component/Implementation/SynchronizedObject.h
        Session/Component/Implementation/SynchronizedObject.cpp
        Socket/dto/MapInitDto.cpp
        Socket/dto/MapInitDto.h
        Session/Component/Implementation/PlayerGenerateManager.cpp
        Session/Component/Implementation/PlayerGenerateManager.h
        Session/Component/Implementation/PlayerComponent.cpp
        Session/Component/Implementation/PlayerComponent.h
        Socket/dto/LoadingProgressDto.h
        Socket/dto/AssignResponseDto.cpp
        Socket/dto/AssignResponseDto.h
        Socket/dto/GenerateDto.h
        Socket/dto/BroadcastPlayerMoveDto.h
        Session/Game/data/CharacterRegistry.h
        Session/Game/data/CharacterRegistry.cpp
        Socket/dto/RespawnDto.cpp
        Socket/dto/RespawnDto.h
        Socket/dto/HitDto.h
        Socket/dto/DeathDto.h
        Session/Game/LifeState.h
        Session/Game/data/WeaponRegistry.cpp
        Session/Game/data/WeaponRegistry.h
        Session/Component/Implementation/WeaponInventory.cpp
        Session/Component/Implementation/WeaponInventory.h
        Socket/dto/DropWeaponDto.h
        Socket/dto/SwapWeaponDto.h
        Socket/dto/ReloadDto.h
        Socket/dto/GetWeaponNotifyDto.h
        Socket/dto/DropWeaponNotifyDto.h
        Socket/dto/SwapWeaponNotifyDto.h
        Socket/dto/ReloadNotifyDto.h
        Session/Action.h
        Session/Component/Implementation/Interactable.cpp
        Session/Component/Implementation/Interactable.h
        Socket/dto/InteractDto.h
        Socket/dto/GenerateObjectDto.h
        Socket/dto/JumpDto.h
        Socket/dto/ShotDto.h
        Socket/dto/ShotNotifyDto.h
        Socket/dto/HitThisDto.h
        Socket/dto/HitThisDto.cpp
        Socket/dto/HitStructureDto.h
        Socket/dto/HitStructureDto.cpp
        Socket/dto/PhaseChangeNotifyDto.h
        Socket/dto/GameEndNotifyDto.h
)
target_precompile_headers(fpsProjectServer PUBLIC pch.h)
add_custom_command(TARGET fpsProjectServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Assets
        $<TARGET_FILE_DIR:fpsProjectServer>/Assets
        COMMENT "Copying Assets directory to binary output directory..."
)

add_custom_command(TARGET fpsProjectServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Prefabs"
        "$<TARGET_FILE_DIR:fpsProjectServer>/Prefabs"
)

find_path(ENET_INCLUDE_DIR enet/enet.h)
find_library(ENET_LIBRARY enet)
include_directories(${ENET_INCLUDE_DIR})
target_link_libraries(fpsProjectServer
        PRIVATE
        ${ENET_LIBRARY}
        ws2_32
        winmm
        d3d11
        d3dcompiler
        dxgi
        dxguid
)
if(WIN32)
    target_link_libraries(fpsProjectServer PRIVATE psapi)
endif()
find_package(cpprestsdk CONFIG REQUIRED)
target_link_libraries(fpsProjectServer PRIVATE cpprestsdk::cpprest cpprestsdk::cpprestsdk_zlib_internal cpprestsdk::cpprestsdk_brotli_internal)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(fpsProjectServer PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(fpsProjectServer PRIVATE Rpcrt4)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
target_link_libraries(fpsProjectServer PRIVATE unofficial::concurrentqueue::concurrentqueue)


