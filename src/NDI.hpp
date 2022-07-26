
#include <cstdlib>
#include <iostream>
#include <string>

#include "ndi/Processing.NDI.Lib.h"

using namespace std::literals;

#if defined(__unix__) || defined(__unix) ||                                    \
    (defined(__APPLE__) && defined(__MACH__))
#define UNIX
#endif

#if defined(UNIX)
#include <dlfcn.h>
#elif defined(WIN32)
#include <windows.h>

#include <AtlBase.h>
#include <AtlConv.h>
#endif

class NDIlib {
private:
#if defined(UNIX)
  void *
#elif defined(WIN32)
  HMODULE
#endif
      dl;

  NDIlib_v5 const *lib;

public:
  NDIlib() {
#if defined(__APPLE__)
    auto dir = "/usr/local/lib/"s;
#else
    auto dir = std::string{std::getenv(NDILIB_REDIST_FOLDER)} + "/";
#endif
    auto path = dir + NDILIB_LIBRARY_NAME;

#if defined(UNIX)
    dl = dlopen(path.c_str(), RTLD_LAZY);
#elif defined(WIN32)
    dl = LoadLibraryA(path.c_str());
#endif
    if (!dl) {
      std::cerr << "Can't find NDI lib, expected at: " << path << '\n';
#if defined(UNIX)
      std::cerr << "Error: " << dlerror() << '\n';
#endif
#if defined(WIN32)
      MessageBox(nullptr, CA2T(path.c_str()), L"Can't find NDI lib", MB_OK);
#endif
      std::terminate();
    }
    lib = reinterpret_cast<decltype(&NDIlib_v5_load)>(
#if defined(UNIX)
        dlsym
#elif defined(WIN32)
        GetProcAddress
#endif
        (dl, "NDIlib_v5_load"))();
    if (lib == nullptr) {
      std::cerr << "Can't find NDI lib\n";
      std::terminate();
    }
  }

  ~NDIlib() {
#if defined(UNIX)
    dlclose(dl);
#elif defined(WIN32)
    FreeLibrary(dl);
#endif
  }

  auto operator*() const -> NDIlib_v5 const & { return *lib; }
  auto operator->() const -> NDIlib_v5 const * { return lib; }
};
