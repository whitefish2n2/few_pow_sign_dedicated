//
// Created by white on 26. 5. 8..
//

#ifndef FPSPROJECTSERVER_MONITORUTIL_H
#define FPSPROJECTSERVER_MONITORUTIL_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// --- OS별 필요한 헤더 선언 ---
#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h> // 메모리 정보 획득용 (CMake에서 Psapi.lib 링킹 필요)
#elif defined(__linux__)
    #include <unistd.h>
#endif

// --- 메모리 사용량 (MB) 반환 ---
inline long long GetProcessMemoryUsageMB() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        // WorkingSetSize: 현재 프로세스가 RAM에 물리적으로 차지하고 있는 크기 (Byte)
        return pmc.WorkingSetSize / (1024 * 1024);
    }
    return 0;

#elif defined(__linux__)
    // Linux는 /proc/self/status 에서 VmRSS(실제 물리 메모리 사용량)를 읽어옵니다.
    std::ifstream file("/proc/self/status");
    std::string line;
    while (std::getline(file, line)) {
        if (line.compare(0, 6, "VmRSS:") == 0) {
            std::istringstream iss(line.substr(6));
            long long rssKb;
            iss >> rssKb;
            return rssKb / 1024; // KB -> MB
        }
    }
    return 0;

#else
    return 0; // 기타 OS 미지원
#endif
}

// --- CPU 사용량 (%) 반환 ---
inline double GetProcessCpuUsage() {
#ifdef _WIN32
    static FILETIME prevSysIdle, prevSysKernel, prevSysUser;
    static FILETIME prevProcCreation, prevProcExit, prevProcKernel, prevProcUser;
    static bool firstRun = true;

    FILETIME sysIdle, sysKernel, sysUser;
    FILETIME procCreation, procExit, procKernel, procUser;

    GetSystemTimes(&sysIdle, &sysKernel, &sysUser);
    GetProcessTimes(GetCurrentProcess(), &procCreation, &procExit, &procKernel, &procUser);

    if (firstRun) {
        prevSysIdle = sysIdle; prevSysKernel = sysKernel; prevSysUser = sysUser;
        prevProcKernel = procKernel; prevProcUser = procUser;
        firstRun = false;
        return 0.0; // 첫 호출 시점에는 기준점이 없으므로 0을 반환
    }

    // FILETIME을 64비트 정수로 변환하는 람다 함수
    auto ft2ull = [](const FILETIME& ft) -> unsigned long long {
        ULARGE_INTEGER uli;
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        return uli.QuadPart;
    };

    unsigned long long sysKernelDiff = ft2ull(sysKernel) - ft2ull(prevSysKernel);
    unsigned long long sysUserDiff = ft2ull(sysUser) - ft2ull(prevSysUser);
    unsigned long long procKernelDiff = ft2ull(procKernel) - ft2ull(prevProcKernel);
    unsigned long long procUserDiff = ft2ull(procUser) - ft2ull(prevProcUser);

    // 전체 시스템의 경과 시간 (Kernel 타임에 Idle이 포함됨)
    unsigned long long totalSys = sysKernelDiff + sysUserDiff;
    // 현재 프로세스가 사용한 시간
    unsigned long long totalProc = procKernelDiff + procUserDiff;

    prevSysIdle = sysIdle; prevSysKernel = sysKernel; prevSysUser = sysUser;
    prevProcKernel = procKernel; prevProcUser = procUser;

    if (totalSys > 0) {
        return ((double)totalProc / totalSys) * 100.0;
    }
    return 0.0;

#elif defined(__linux__)
    static unsigned long long prevTotalTime = 0;
    static unsigned long long prevProcTime = 0;
    static bool firstRun = true;

    // 1. 전체 시스템 CPU 시간 가져오기
    std::ifstream statFile("/proc/stat");
    std::string line;
    std::getline(statFile, line);
    std::istringstream iss(line);
    std::string dummy;
    iss >> dummy; // "cpu" 글자 스킵
    unsigned long long totalTime = 0, val;
    while (iss >> val) {
        totalTime += val;
    }
    statFile.close();

    // 2. 현재 프로세스의 CPU 시간 가져오기
    std::ifstream procStatFile("/proc/self/stat");
    std::getline(procStatFile, line);
    procStatFile.close();

    // 프로세스 이름에 띄어쓰기가 있을 수 있으므로 ')' 위치를 찾아 이후부터 파싱
    size_t rparen = line.find_last_of(')');
    unsigned long long utime = 0, stime = 0;
    if (rparen != std::string::npos) {
        std::istringstream procIss(line.substr(rparen + 2));
        for (int i = 1; i <= 11; ++i) { // utime(14번째 항목) 직전까지 스킵
            procIss >> dummy;
        }
        procIss >> utime >> stime;
    }
    unsigned long long procTime = utime + stime;

    if (firstRun) {
        prevTotalTime = totalTime;
        prevProcTime = procTime;
        firstRun = false;
        return 0.0; // 첫 호출 시 기준점 세팅
    }

    unsigned long long totalDiff = totalTime - prevTotalTime;
    unsigned long long procDiff = procTime - prevProcTime;

    prevTotalTime = totalTime;
    prevProcTime = procTime;

    if (totalDiff > 0) {
        // 코어 수만큼 곱해주어 멀티코어 환경에서 올바른 %를 나타내도록 보정
        return ((double)procDiff / (double)totalDiff) * 100.0 * sysconf(_SC_NPROCESSORS_ONLN);
    }
    return 0.0;

#else
    return 0.0; // 기타 OS 미지원
#endif
}
#endif //FPSPROJECTSERVER_MONITORUTIL_H