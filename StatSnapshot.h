//
// Created by white on 26. 9. 18..
//

#ifndef FPSPROJECTSERVER_STATSNAPSHOT_H
#define FPSPROJECTSERVER_STATSNAPSHOT_H
struct StatSnapshot {
    double processCpu = 0.0;
    long long processMemoryBytes = 0;
    long long pcMemoryBytes = 0;
    long long pcMemoryMaxBytes = 0;
    size_t sessionCount = 0;
    int connectedPlayers = 0;
    int liveSessionCount = 0;

    long long avgTps = 0, avgTickUs = 0, avgLagMs = 0;
    double avgThreadCpu = 0.0;
    long long avgBroadphaseUs = 0, avgPairsChecked = 0, avgPairsHit = 0;
    long long avgObjectsAtStart = -1;
    int sessionsWithObjectCount = 0;

    long long avgEventQueueUs = 0, avgUpdateComponentsUs = 0, avgFlushGameObjectUs = 0;
    long long avgBroadcastMovementsUs = 0, avgBroadcastObjectMovementsUs = 0, avgCheckDisconnectedUs = 0;

    long long avgPhysicsIntegrateUs = 0, avgStaticOverlapUs = 0, avgStaticPairsFound = 0, avgNarrowPhaseUs = 0;
    long long avgNarrowPhaseStaticUs = 0, avgNarrowPhaseDynamicUs = 0;
};

#endif //FPSPROJECTSERVER_STATSNAPSHOT_H