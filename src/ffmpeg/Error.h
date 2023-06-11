﻿//
// Created by 熊朝伟 on 2023-06-06.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class Error {
public:
    int32_t code;
    std::string message;
    std::string method;
    int32_t line;

    void print();

    static bool verify(int error, const char* method, int line);
    static void report(int error, const char* method, int line);
};

OMP_FFMPEG_NAMESPACE_END
