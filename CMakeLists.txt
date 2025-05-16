cmake_minimum_required(VERSION 3.15)

project(fpsProjectServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(fpsProjectServer
        main.cpp
        Session/GameSession.cpp
        Session/GameSession.h
        util/util.cpp
        util/util.h
        http-client/DedicateServerNotifier.cpp
        http-client/DedicateServerNotifier.h
        server-status/ServerStat.cpp
        server-status/ServerStat.h
        http-listener/httpRestClient.cpp
        http-listener/httpRestClient.h
        Session/Game/Player.h
        Session/Game/PlayerStatus.h
        Session/Dto/Weapons.h
        Session/Dto/NetworkStatus.h
        Session/FhishiX/Vector3.h
        Session/Dto/SessionStatus.h
        Session/netcode/SessionNetworkDto.h
        Session/netcode/SessionNetworkDto.cpp
        Session/sessionPool/SessionManager.cpp
        Session/sessionPool/SessionManager.h
        Session/netcode/CreateSessionNetworkDto.cpp
        Session/netcode/CreateSessionNetworkDto.h
        Session/Dto/GameMode.h
        Session/netcode/newPlayerDto.h
        Constants.h
        Socket/EnetClient.cpp
        Socket/EnetClient.h
        Socket/dto/SocketEventType.h
        Socket/dto/AssignDto.h
        Socket/dto/AssignDto.cpp
        Socket/dto/MoveDto.cpp
        Socket/dto/MoveDto.h
        Session/FhishiX/Vector2.h
        Socket/dto/DefaultDto.cpp
        Socket/dto/DefaultDto.h
        Session/FhishiX/Vertex.h
        Session/Dto/MapEnum.h
        Session/Game/Player.cpp
        Session/Game/SimulationMap.cpp
        Session/Game/SimulationMap.h
        Session/Game/Map.h
        Session/Game/MapManager.cpp
        Session/Game/MapManager.h
        Session/Game/Map.cpp
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
