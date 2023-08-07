//
// Created by 熊朝伟 on 2023-06-06.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

static std::string __av_err2str(int error) {
    char message[AV_ERROR_MAX_STRING_SIZE] = { 0 };
    return av_make_error_string(message, AV_ERROR_MAX_STRING_SIZE, error);
}

bool Error::verify(int error, const char* method, int line) {
    if (error < 0) {
        report(error, method, line);
        return false;
    }
    return true;
}

void Error::report(int error, const char* method, int line) {
    std::string message = __av_err2str(error);
    printf("Error:\n\tfrom: %s(%d)\n\tcode: %d\n\tmessage: %s\n", method, line, error, message.c_str());
}

AVError::AVError(int error, const char* method, int line) {
    this->code = error;
    this->message = __av_err2str(error);
    this->method = method;
    this->line = line;
}

RefPtr<Error> AVError::from(int error, const char* method, int line) {
    return error < 0 ? new AVError(error, method, line) : nullptr;
}
