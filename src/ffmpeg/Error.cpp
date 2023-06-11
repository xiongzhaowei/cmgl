//
// Created by 熊朝伟 on 2023-06-06.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

static std::string __av_err2str(int error) {
    char message[AV_ERROR_MAX_STRING_SIZE] = { 0 };
    return av_make_error_string(message, AV_ERROR_MAX_STRING_SIZE, error);
}

void Error::print() {
    printf("Error:\n\tfrom: %s(%d)\n\tcode: %d\n\tmessage: %s", method.c_str(), line, code, message.c_str());
}

bool Error::verify(int error, const char* method, int line) {
    if (error < 0) {
        report(error, method, line);
        return false;
    }
    return true;
}

void Error::report(int error, const char* method, int line) {
    Error e;
    e.code = error;
    e.message = __av_err2str(error);
    e.method = method;
    e.line = line;

    e.print();
}
