
/**
 * Qentem Template Test
 *
 * @brief     For testing Qentem Template.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include <Extension/Template.hpp>
#include <ctime>
#include <fstream>
#include <iostream>

using Qentem::Array;
using Qentem::Document;
using Qentem::String;

String   read_file(const char *fullpath) noexcept;
Document get_document() noexcept;

void run() noexcept {

    Document data;

    // Template
    // data = get_document();
    // Or
    data = Document::FromJSON(read_file("./Test/temp.json"));
    std::wcout << Qentem::Template::Render(read_file("./Test/temp.qtml"), &data).Str << '\n';
    // std::wcout << data.ToJSON().Str << '\n';

    // std::wcout << "Importing...";
    // UNumber took = static_cast<UNumber>(clock());
    // data         = Document::FromJSON(read_file("./Test/bigjson.json"));
    // // Qentem::Engine::Search(read_file("./Test/bigjson.json"), Document::GetJsonExpres());
    // took = (static_cast<UNumber>(clock()) - took);
    // std::wcout << Qentem::String::FromNumber((static_cast<double>(took) / CLOCKS_PER_SEC), 2, 3).Str << ' ' << '\n';

    // std::wcout << "Exporting...";
    // took = static_cast<UNumber>(clock());
    // data.ToJSON();
    // took = (static_cast<UNumber>(clock()) - took);
    // std::wcout << "Done! ";
    // std::wcout << Qentem::String::FromNumber((static_cast<double>(took) / CLOCKS_PER_SEC), 2, 3).Str << '\n';
}

int main() noexcept {
    // for (UNumber y = 0; y < 100; y++) {
    //     run();
    // }

    run();

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

Document get_document() noexcept {
    Document data = Document();

    data[L"var1"]  = L"\"1\"";
    data[L"PP"]    = L"gg";
    data[L"nu"]    = nullptr;
    data[L"bool"]  = false;
    data[L"bool2"] = true;
    data[L"&&"]    = 1000.0;

    data[L"var2"]       = L"11";
    data[L"var2"]       = L"2"; // Testing override .
    data[L"var3"]       = L"3";
    data[L"var4"]       = L"4";
    data[L"var5"]       = L"5";
    data[L"num34"]      = Array<String>().Add(L'3').Add(L'4');
    data[L"var_string"] = L"image";
    data[L"engine"]     = L"Qentem";
    data[L"abc1"]       = Array<String>().Add(L'B').Add(L'C').Add(L'D').Add(L'A');
    data[L"abc2"]       = Array<String>().Add(L'E').Add(L'F').Add(L'A');
    data[L"numbers"]    = Array<double>().Add(0).Add(1).Add(2).Add(3).Add(4).Add(5);
    data[L"empty"]      = L"";
    data[L"math"] = L"((2* (1 * 3)) + 1 - 4) + (((10 - 5) - 6 + ((1 + 1) + (1 + 1))) * (8 / 4 + 1)) - (1) - (-1) + 2";

    data[L"abc"]         = Document();
    data[L"abc"][L'B']   = L"b";
    data[L"abc"][L"@@"]  = 100.0;
    data[L"abc"][L'A']   = L"a";
    data[L"abc"][L'C']   = L"c";
    data[L"abc"][L'D']   = L"d";
    data[L"abc"][L'E']   = Array<String>().Add(L'O').Add(L"K!");
    data[L"abc"][L"A-Z"] = L"ABCDEFGHIGKLMNOBQRST.....";

    data[L"multi"]                = Document();
    data[L"multi"][L"arr1"]       = Document();
    data[L"multi"][L"arr1"][L'E'] = Array<String>().Add(L'O').Add(L"K!");
    data[L"multi"][L"arr2"]       = Array<String>().Add(L'B').Add(L'C').Add(L'D').Add(L'A');
    data[L"multi"][L'C']          = L"cool";

    return data;
}
