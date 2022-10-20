#include "result.h"

namespace val {
    val::Error error(val::ErrorCode errorCode, std::string errorMessage) {
        return {errorCode, std::move(errorMessage)};
    }
}

