// this_file: src/exit_codes.h
// Exit code constants for fontlift-win-cli
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

#ifndef EXIT_CODES_H
#define EXIT_CODES_H

// Exit codes used throughout the application
constexpr int EXIT_SUCCESS_CODE = 0;        // Operation successful
constexpr int EXIT_ERROR = 1;                // General error (file not found, operation failed, etc.)
constexpr int EXIT_PERMISSION_DENIED = 2;    // Administrator privileges required

#endif // EXIT_CODES_H
