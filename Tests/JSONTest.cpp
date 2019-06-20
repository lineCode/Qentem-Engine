#include <Extension/Document.hpp>
#include <ctime>
#include <fstream>
#include <iostream>

using Qentem::Array;
using Qentem::Document;
using Qentem::String;

String read_file(const char *fullpath) noexcept;

void run() noexcept {

    UNumber took = 0;
    std::wcout << "Importing... ";
    took            = static_cast<UNumber>(clock());
    Document &&data = Document::FromJSON(read_file("./Tests/bigjson.json"));
    // Qentem::Engine::Search(read_file("./Tests/bigjson.json"), Document::GetJsonExpres());
    took = (static_cast<UNumber>(clock()) - took);
    std::wcout << Qentem::String::FromNumber((static_cast<double>(took) / CLOCKS_PER_SEC), 2, 3, 3).Str << ' ';

    // std::wcout << L'\n';

    std::wcout << "Exporting... ";
    took = static_cast<UNumber>(clock());
    data.ToJSON();
    took = (static_cast<UNumber>(clock()) - took);
    std::wcout << Qentem::String::FromNumber((static_cast<double>(took) / CLOCKS_PER_SEC), 2, 3, 3).Str << '\n';

    // std::wcout << data.ToJSON().Str;
}

int main() noexcept {
    for (UNumber y = 1; y <= 10; y++) {
        run();
        // std::wcout << L'\n';
    }

    std::wcout << L'\n';
    // std::getchar();

    return 1;
}

String read_file(const char *fullpath) noexcept {
    String content = L"";

    std::ifstream file(fullpath, std::ios::ate | std::ios::out);
    if (file.is_open()) {
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);
        UNumber u_size = (UNumber(size) + 1);
        char *  _tmp   = new char[u_size];
        file.read(_tmp, size);
        _tmp[(u_size - 1)] = L'\0';

        content = String(_tmp);
        delete[] _tmp;
        file.close();
    } else {
        std::wcout << '\n' << fullpath << L" does not exist!\n";
    }

    return content;
}
