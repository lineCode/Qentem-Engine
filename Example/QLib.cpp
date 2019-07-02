#include <Extension/Template.hpp>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

using Qentem::Document;
using Qentem::String;
using Qentem::Template;
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
#endif

extern "C" {
DllExport wchar_t *renderTemplate_w(wchar_t const *temp, wchar_t const *json, bool comments) {
    Document &&data     = Document::FromJSON(json, comments);
    String &&  rendered = Template::Render(temp, &data);
    data.Reset();

    return rendered.Eject();
}
}

extern "C" {
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif

DllExport char *renderTemplate(char const *temp, char const *json, bool comments) {
    String &&  S_json   = String(json);
    Document &&data     = Document::FromJSON(S_json, comments);
    String &&  rendered = Template::Render(temp, &data);

    data.Reset();
    S_json.Reset();

    if (rendered.Length == 0) {
        rendered = L"";
    }

    char *tmp = new char[rendered.Length + 1]; // 1 for '\0'
    for (UNumber j = 0; j <= rendered.Length; j++) {
        // It got it as a "char" type in the first place, so there wont be any above 255.
        tmp[j] = static_cast<char>(static_cast<unsigned int>(rendered[j]));
    }

    return tmp;
}
}
