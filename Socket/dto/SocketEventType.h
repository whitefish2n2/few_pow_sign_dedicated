//
// Created by white on 25. 5. 9.
//

#ifndef SOCKETEVENTTYPE_H
#define SOCKETEVENTTYPE_H

enum class SocketEventType : unsigned char
{
    Assign = 0, //P2S _R_
    Input = 1,//P2S
    Move = 2,//P2S

    Setup = 3, // S2P_R_
    Update = 4,//S2P
    NotUse = 5,   //S2P _R_
    NotUseTwo = 6,  //S2P _R_
    GeneratePlayer = 7, //S2P _R_

    MapInit = 8,//S2P_R_

    MapInitReady = 9,//P2S _R_

    Progress = 10, //P2S _R_
    ProgressNotify = 11, // S2P

    AssignResponse = 12, //S2P _R_

    PlayerMove = 13, //S2P
    ObjectMove = 14, //S2P
    RespawnPlayer = 15,//S2P _R_
    Death            = 16, //S2P _R_
    Interact        = 17, //P2S _R_
    // ── 무기 시스템 (19b-2 / 19c) ──
    GetWeaponNotify  = 18, //S2P _R_
    DropWeapon       = 19, //P2S _R_
    DropWeaponNotify = 20, //S2P _R_
    SwapWeapon       = 21, //P2S _R_
    SwapWeaponNotify = 22, //S2P _R_
    Reload           = 23, //P2S _R_
    ReloadNotify     = 24, //S2P _R_
    Shot             = 25, //P2S
    ShotNotify       = 26, //S2P _R_
    HitThis          = 27, //P2S _R_
    HitNotify        = 28, //S2P _R_

    GenerateObject = 29, //S2P _R_
    Jump = 30, //P2S _R_

    PhaseChangeNotify = 31, //S2P _R_
    GameEndNotify     = 32, //S2P _R_
    RoundEndNotify    = 33, //S2P _R_
    HitStructure      = 34, //P2S _R_ — 플레이어 아닌 구조물(벽 등) 명중 클레임, GameObject id로 지칭

    Ping = 252,//S2P
    Pong = 253,//P2S

    Default = 254,
};

#include <enet/enet.h>
inline enet_uint32 GetPacketFlags(SocketEventType type) {
    switch (type) {
        case SocketEventType::PlayerMove:
        case SocketEventType::ObjectMove:
        case SocketEventType::Update:
        case SocketEventType::ProgressNotify:
            return ENET_PACKET_FLAG_UNSEQUENCED;
        default:
            return ENET_PACKET_FLAG_RELIABLE;
    }
}

#endif //SOCKETEVENTTYPE_H
