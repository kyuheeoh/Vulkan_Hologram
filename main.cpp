
#include <string>
#include <vector>
#include "moleculeviewer.h"
#include "build_options.h"
namespace
{

    Application *create_app(const std::vector<std::string> &args)
    {
        return new MoleculeViewer(args);
    }

    Application *create_app(int argc, char **argv)
    {
        std::vector<std::string> args(argv, argv + argc);
        return create_app(args);
    }

} // namespace

#ifdef VK_USE_PLATFORM_XCB_KHR

#include "ShellXcb.h"

int main(int argc, char **argv)
{
   Application *application = create_app(argc, argv);
    {
        ShellXcb shell(*application);
        shell.run();
    }
    delete application;

    return 0;
}

#elif defined(VK_USE_PLATFORM_ANDROID_KHR)

#include <android/log.h>
#include "ShellAndroid.h"

void android_main(android_app *app)
{
    Game *game = create_game(ShellAndroid::get_args(*app));
    try {
        ShellAndroid shell(*app, *game);
        shell.run();
    } catch (const std::runtime_error &e) {
        __android_log_print(ANDROID_LOG_ERROR, game->settings().name.c_str(),
                "%s", e.what());
    }

    delete game;
}

#elif defined(VK_USE_PLATFORM_WIN32_KHR)

#include "ShellWin32.h"

int main(int argc, char **argv)
{
    Game *game = create_game(argc, argv);
    {
        ShellWin32 shell(*game);
        shell.run();
    }
    delete game;

    return 0;
}

#endif // VK_USE_PLATFORM_XCB_KHR
