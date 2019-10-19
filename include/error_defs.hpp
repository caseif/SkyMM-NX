#pragma once

#define FATAL(fmt, ...) CONSOLE_CLEAR_SCREEN(); \
                        CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_RED); \
                        printf("Fatal error: " ); \
                        CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_YELLOW); \
                        printf(fmt, ##__VA_ARGS__); \
                        CONSOLE_MOVE_DOWN(3); \
                        CONSOLE_MOVE_LEFT(99); \
                        CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_GREEN); \
                        printf("Press (+) to exit.\n")

#define FATAL_CODE(code, fmt, ...) FATAL(fmt " (code %d)", ##__VA_ARGS__, code)

#define PANIC() FATAL("Panic @ %s:%d\n\nPlease contact the developer", __FILE__, __LINE__)

#define RC_SUCCESS(rc) (rc == 0)
#define RC_FAILURE(rc) (rc != 0)

#define DO_OR_DIE(rc, task, fail_fmt, ...)  if ((rc = RC_FAILURE((task)))) { \
                                                FATAL_CODE(rc, fail_fmt, ##__VA_ARGS__); \
                                                return -1; \
                                            }
