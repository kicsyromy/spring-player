#ifndef SPRING_PLAYER_UTILITY_GLOBAL_H
#define SPRING_PLAYER_UTILITY_GLOBAL_H

#define void_p(ptr) static_cast<const void *>(ptr)

#if __cplusplus >= 201703L
#define SPRING_FALL_THROUGH [[fallthrough]]
#else
#define SPRING_FALLTHROUGH
#endif

#endif // !SPRING_PLAYER_UTILITY_GLOBAL_H
