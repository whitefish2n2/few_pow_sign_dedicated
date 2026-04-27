//
// Created by white on 26. 4. 24..
//

#ifndef FPSPROJECTSERVER_STRINGUTIL_H
#define FPSPROJECTSERVER_STRINGUTIL_H
#pragma once
#include <string>
#include <string_view>

namespace StringUtils {
    inline std::string Trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return ""; // 전부 공백인 경우 빈 문자열 반환
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

}
#endif //FPSPROJECTSERVER_STRINGUTIL_H