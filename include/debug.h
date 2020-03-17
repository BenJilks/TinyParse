#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 1

#if DEBUG

#define _LOG(...) { printf(__VA_ARGS__); }
#define LOG(...) { debug_prefix(); _LOG(__VA_ARGS__); }
void debug_prefix();
void debug_start_scope();
void debug_end_scope();

#else

#define _LOG(...) ;
#define LOG(...) ;
static void debug_prefix() {}
static void debug_start_scope() {}
static void debug_end_scope() {}

#endif

#endif // DEBUG_H
