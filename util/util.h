//
// Created by user on 25. 4. 11.
//

#pragma once

#include <string>

void Log(const std::string& message);
std::string GetLocalIP();

std::string GenerateUuid();
std::string hash(std::string value);
std::string wstring_to_utf8(const std::wstring& wstr);