#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 1

#if DEBUG

#define LOG(...) { debug_prefix(); printf(__VA_ARGS__); }
void debug_prefix();
void debug_start_scope();
void debug_end_scope();

#else
#define LOG(...) ;
void debug_prefix() {}
void debug_start_scope() {}
void debug_end_scope() {}
#endif

#endif // DEBUG_H
