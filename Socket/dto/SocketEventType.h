//
// Created by white on 25. 5. 9.
//

#ifndef SOCKETEVENTTYPE_H
#define SOCKETEVENTTYPE_H

enum SocketEventType:unsigned char
{
    //player->server
    Assign=0,//_R_
    Input=1,
    Move=2,

    //server->player
    Setup=3,//_R_
    Update=4,
    Hit=5,//_R_
    Swap=6,//_R_
    Generate=7,//_R_



    //fuck that
    Default = 254
};
#endif //SOCKETEVENTTYPE_H
