#include <Extension/Template.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

using Qentem::Document;
using Qentem::String;

#ifdef _WIN32
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    // switch (ul_reason_for_call) {
    //     case DLL_PROCESS_ATTACH:
    //     case DLL_THREAD_ATTACH:
    //     case DLL_THREAD_DETACH:
    //     case DLL_PROCESS_DETACH:
    //         break;
    // }
    return TRUE;
}
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

extern "C" {

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif

DllExport char *qentem_render_template(const char *temp, const char *json, bool comments) {
    Document data = Document::FromJSON(json, 0, String::Count(json), comments);
    return Qentem::Template::Render(temp, 0, String::Count(temp), &data).Eject();
}
}
