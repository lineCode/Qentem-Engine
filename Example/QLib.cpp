#include <Extension/Template.hpp>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

using Qentem::Document;
using Qentem::String;
using Qentem::UNumber;

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
DllExport wchar_t *renderTemplate_w(wchar_t const *temp, wchar_t const *json, bool comments) {
    Document data     = Document::FromJSON(json, comments);
    String   rendered = Qentem::Template::Render(temp, &data);
    data.Reset();

    return rendered.Eject();
}
}

extern "C" {
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif

DllExport char *renderTemplate(char const *temp, char const *json, bool comments) {
    Document data     = Document::FromJSON(String(json), comments);
    String   rendered = Qentem::Template::Render(temp, &data);
    data.Reset();

    if (rendered.Length == 0) {
        rendered = L"";
    }

    char *tmp = new char[rendered.Length + 1]; // 1 for '\0'
    for (UNumber j = 0; j <= rendered.Length; j++) {
        // It got it as a "char" type in the first place, so there won't be any above 255.
        tmp[j] = static_cast<char>(rendered[j]);
    }

    return tmp;
}
}
