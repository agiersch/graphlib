#ifndef SIMPLE_DW_H
#define SIMPLE_DW_H

#include <string>

void drawText(int x, int y, const char *text, int flags = 0);
void drawText(int x, int y, const std::string &text, int flags = 0);

#define SDW_CHECK_ARGS(A, B, ...) SDW_CHECK_ARGS_(__VA_ARGS__, B, A)
#define SDW_CHECK_ARGS_(a2, a1, X, ...) X

#define call0()                                         \
    simpleDW_user_main()

#define call2()                                         \
    extern int global_argc;                             \
    extern char **global_argv;                          \
    simpleDW_user_main(global_argc, global_argv)

#define main(...)                                         \
    simpleDW_user_main(__VA_ARGS__);                      \
    void simpleDW_user_main_wrapper()                     \
    {                                                     \
        SDW_CHECK_ARGS(call0(), call2(), __VA_ARGS__);    \
    }                                                     \
    int simpleDW_user_main(__VA_ARGS__)

#endif // !SIMPLE_DW_H
