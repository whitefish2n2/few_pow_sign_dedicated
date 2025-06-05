cmake_minimum_required(VERSION 3.15)

project(fpsProjectServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
        Session/Game/SimulationMap.cpp
        Session/Game/Map.cpp
        Session/Game/MapManager.cpp
        Session/FhishiX/ObjectTag.h
        Session/FhishiX/triangle.h
        Session/FhishiX/GameObject.h
        Session/Game/BspTree.h
        Session/FhishiX/AABB.h
        Session/FhishiX/FhishiX.h
        Session/Game/BspTree.cpp
        Socket/BroadcastMoveDto.cpp
        Socket/BroadcastMoveDto.h
        Socket/Game/Gun.h
        Socket/Game/InteractableObject.h
        Session/FhishiX/Layer.h
        Session/Game/Collider.cpp
        Session/Game/Collider.h
        Session/Game/Property.cpp
        Session/Game/Property.h
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
