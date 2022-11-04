#include <QApplication>
#include <DrawingWindow.h>

extern void simpleDW_user_main_wrapper();

namespace {
    DrawingWindow *global_w;
    int global_argc;
    char **global_argv;

    void drawing_function(DrawingWindow& w)
    {
        global_w = &w;
        simpleDW_user_main_wrapper();
    }
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    global_argc = argc;
    global_argv = argv;
    DrawingWindow window(drawing_function);
    window.show();
    return application.exec();
}

#include <SimpleDW.h>

void drawText(int x, int y, const char *text, int flags)
{
    global_w->drawText(x, y, text, flags);
}

void drawText(int x, int y, const std::string &text, int flags)
{
    global_w->drawText(x, y, text, flags);
}
