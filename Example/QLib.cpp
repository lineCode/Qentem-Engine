#include <Extension/Template.hpp>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

using Qentem::Document;
using Qentem::String;
using Qentem::Template;

extern "C" {
const wchar_t *renderTemplate_w(const wchar_t *temp, const wchar_t *json, bool comments) {
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

const char *renderTemplate(const char *temp, const char *json, bool comments) {
    String &&  S_json = String(json);
    Document &&data   = Document::FromJSON(S_json, comments);
    String &&  rend   = Template::Render(temp, &data);

    data.Reset();
    S_json.Reset();

    if (rend.Length == 0) {
        rend = L"";
    }

    char *tmp = new char[rend.Length + 1]; // 1 for '\0'
    for (UNumber j = 0; j <= rend.Length; j++) {
        // It got it as a "char" type in the first place, so there wont be any above 255.
        tmp[j] = static_cast<char>(static_cast<unsigned int>(rend.Str[j]));
    }

    return tmp;
}
}
