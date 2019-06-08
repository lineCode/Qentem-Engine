#include <Extension/Template.hpp>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

using Qentem::Document;
using Qentem::String;
using Qentem::Template;

extern "C" {
const wchar_t *renderTemplate_w(const wchar_t *temp, const wchar_t *json, bool comment) {
    Document &&data = Document::FromJSON(json, comment);

    String rendered = Template::Render(temp, &data);

    auto _str    = rendered.Str;
    rendered.Str = nullptr;

    return _str;
}
}

extern "C" {
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif

const char *renderTemplate(const char *temp, const char *json, bool comment) {
    String     S_json = String(json);
    Document &&data   = Document::FromJSON(S_json, comment);
    String &&  rend   = Template::Render(temp, &data);

    char *tmp = new char[rend.Length + 1]; // 1 for /0
    for (UNumber j = 0; j <= rend.Length; j++) {
        // It got it as a "char" type in the first place, so there wont be any above 255.
        tmp[j] = rend.Str[j];
    }
    return tmp;
}
}
