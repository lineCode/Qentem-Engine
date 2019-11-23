#include <Extension/Template.hpp>
#include <iostream>

using Qentem::Document;
using Qentem::String;

int main() {
    Document data = Document::FromJSON(R"([10,"String", true, false, null])");

    String content = R"(<loop value="s_value">s_value </loop>{v:0})";

    std::cout << "Template2:\n" << Qentem::Template::Render(content, &data).Str << '\n';
}
