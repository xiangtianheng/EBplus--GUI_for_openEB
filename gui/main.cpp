// gui/main.cpp — application entry point.

#include <QApplication>
#include <QFont>
#include <QSurfaceFormat>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#ifdef __APPLE__
#include <filesystem>
#include <mach-o/dyld.h>
#include <vector>

namespace {

std::string path_list(const std::vector<std::filesystem::path>& paths) {
    std::string value;
    for (const auto& path : paths) {
        if (path.empty() || !std::filesystem::exists(path)) continue;
        if (!value.empty()) value += ':';
        value += path.string();
    }
    return value;
}

void set_env_if_unset(const char* name, const std::string& value) {
    if (!std::getenv(name) && !value.empty()) {
        ::setenv(name, value.c_str(), 0);
    }
}

std::filesystem::path bundle_contents_dir(const char* argv0) {
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string buffer(size, '\0');
    std::filesystem::path executable = argv0;
    if (_NSGetExecutablePath(buffer.data(), &size) == 0) {
        buffer.resize(std::strlen(buffer.c_str()));
        executable = buffer;
    }
    const auto exe_dir = std::filesystem::weakly_canonical(executable).parent_path();
    if (exe_dir.filename() == "MacOS" && exe_dir.parent_path().filename() == "Contents") {
        return exe_dir.parent_path();
    }
    return {};
}

} // namespace
#endif

#include "main_window.h"

// Sets default OpenEB runtime env vars only if the user has not already
// configured them.  This allows users with non-standard HAL plugin locations
// (e.g. CenturyArks cameras) to simply export MV_HAL_PLUGIN_PATH in their
// shell and have the GUI respect it.
static void ensure_openeb_env_defaults([[maybe_unused]] const char* argv0) {
    // HAL plugin path — where the SDK looks for camera driver .so files.
    // Default: Prophesee openEB install location. Override by exporting
    // MV_HAL_PLUGIN_PATH before launching the app.
#ifdef __APPLE__
    const auto contents_dir = bundle_contents_dir(argv0);
    set_env_if_unset("MV_HAL_PLUGIN_PATH",
                     path_list({contents_dir / "PlugIns" / "metavision" / "hal" / "plugins"}));
    set_env_if_unset("MV_HAL_PLUGIN_SEARCH_MODE", "PLUGIN_PATH_ONLY");
    set_env_if_unset("MV_HAL_INSTALL_PATH",
                     path_list({contents_dir / "Resources" / "metavision" / "hal"}));
    set_env_if_unset("HDF5_PLUGIN_PATH", path_list({contents_dir / "PlugIns" / "hdf5"}));
#else
    if (!std::getenv("MV_HAL_PLUGIN_PATH")) {
        ::setenv("MV_HAL_PLUGIN_PATH", "/usr/local/lib/metavision/hal/plugins", 0);
    }
    // HDF5 plugin path — needed for reading HDF5 event files.
    if (!std::getenv("HDF5_PLUGIN_PATH")) {
        ::setenv("HDF5_PLUGIN_PATH", "/usr/local/lib/hdf5/plugin", 0);
    }
#endif
    // Platform plugin on Wayland sessions — a deliberate, documented
    // trade-off (both sides are field-observed on this host):
    //  - PRO xcb: older Qt/driver stacks rendered a black QOpenGLWidget
    //    viewport under the native Wayland plugin; xcb (via XWayland) was
    //    the reliable path.
    //  - CON xcb: while ANOTHER window is fullscreen, XWayland presentation
    //    is throttled and every animated view stutters until that window
    //    closes; native Wayland does not have this problem (verified).
    // Conservative default: keep xcb (a black viewport is a fatal failure,
    // the stutter is not), and tell the user about the wayland escape hatch
    // on the terminal. Any user override is respected (no print then).
    if (!std::getenv("QT_QPA_PLATFORM") && std::getenv("WAYLAND_DISPLAY")) {
        ::setenv("QT_QPA_PLATFORM", "xcb", 0);
        std::fprintf(stderr,
                     "[EBplus] Wayland session: defaulting QT_QPA_PLATFORM=xcb (XWayland).\n"
                     "[EBplus] If animated views stutter while another window is fullscreen,\n"
                     "[EBplus] relaunch with QT_QPA_PLATFORM=wayland.\n");
    }
    // Qt 6 may default to the Vulkan RHI backend and produce a black viewport
    // on this GPU; force OpenGL unless the user has set it.
    if (!std::getenv("QSG_RHI_BACKEND")) {
        ::setenv("QSG_RHI_BACKEND", "opengl", 0);
    }
    // Note: LD_LIBRARY_PATH cannot be set here (the dynamic linker has
    // already resolved shared libraries at process start).  If the SDK
    // libraries are in a non-standard path, set it in the shell or use
    // the run.sh launcher.
}

int main(int argc, char* argv[]) {
    ensure_openeb_env_defaults(argc > 0 ? argv[0] : "");

    // Request a core-profile OpenGL 3.3 context for the display widget.
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setSwapInterval(1); // vsync on
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication app(argc, argv);
    QApplication::setApplicationName("GUI for openEB");
    QApplication::setOrganizationName("GUI-for-openEB");
    QApplication::setApplicationVersion("3.0.0");

    // Global UI font — Inter (design §3.9.1) with platform fallbacks so the
    // typeface stays consistent on systems without Inter installed.
    QFont font(QStringLiteral("Inter"), 10);
    font.setFamilies({QStringLiteral("Inter"),
                      QStringLiteral("Segoe UI"),
                      QStringLiteral("Ubuntu"),
                      QStringLiteral("Noto Sans"),
                      QStringLiteral("Sans Serif")});
    font.setStyleStrategy(QFont::PreferAntialias);
    app.setFont(font);

    gui::MainWindow window;
    window.show();

    return app.exec();
}
