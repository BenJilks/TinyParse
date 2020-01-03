#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 1

#if DEBUG
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...) ;
#endif

#endif // DEBUG_H
