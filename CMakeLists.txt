cmake_minimum_required(VERSION 3.15)

project(fpsProjectServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options(/utf-8)
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
        Socket/BroadcastMoveDto.cpp
        Socket/BroadcastMoveDto.h
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
)

find_path(ENET_INCLUDE_DIR enet/enet.h)
find_library(ENET_LIBRARY enet)
include_directories(${ENET_INCLUDE_DIR})
target_link_libraries(fpsProjectServer
        PRIVATE
        ${ENET_LIBRARY}
        ws2_32
        winmm
)
find_package(cpprestsdk CONFIG REQUIRED)
target_link_libraries(fpsProjectServer PRIVATE cpprestsdk::cpprest cpprestsdk::cpprestsdk_zlib_internal cpprestsdk::cpprestsdk_brotli_internal)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(fpsProjectServer PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(fpsProjectServer PRIVATE Rpcrt4)
