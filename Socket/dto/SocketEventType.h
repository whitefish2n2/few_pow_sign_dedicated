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
    Hit = 5,   //S2P _R_
    Swap = 6,  //S2P _R_
    Generate = 7, //S2P _R_

    MapInit = 8,//S2P_R_

    MapInitReady = 9,//P2S _R_

    Progress = 10, //P2S _R_
    ProgressNotify = 11, // S2P

    AssignResponse = 12, //S2P _R_

    Ping = 252,//S2P
    Pong = 253,//P2S

    Default = 254,
};
#endif //SOCKETEVENTTYPE_H
