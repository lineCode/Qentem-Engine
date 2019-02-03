
/**
 * Qentem Template Test
 *
 * @brief     For testing Qentem Template.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Addon/Template.hpp"
#include <ctime>
#include <fstream>
#include <iostream>

using Qentem::Array;
using Qentem::String;
using Qentem::Template;
using Qentem::Tree;

String read_file(const char fullpath[]) noexcept;
Tree   get_tree() noexcept;

void run() noexcept {
    Tree data;

    // Template
    // data           = get_tree();
    // Or
    // data = Tree::FromJSON(read_file("./Test/temp.json"));

    // std::wcout << Template().Render(read_file("./Test/temp.qtml"), &data).Str;

    std::wcout << "\nImporting JSON...\n";
    UNumber took = clock();
    data         = Tree::FromJSON(read_file("./Test/bigjson.json"));

    std::wcout << "Done! ";
    took = (clock() - took);
    std::wcout << Qentem::String::FromNumber((static_cast<double>(took) / CLOCKS_PER_SEC), 2, 3).Str;
    std::wcout << "\n";

    std::wcout << "\n\nExporting JSON...\n";
    took = clock();
    data.ToJSON();
    took = (clock() - took);
    std::wcout << "Done! ";
    std::wcout << Qentem::String::FromNumber((static_cast<double>(took) / CLOCKS_PER_SEC), 2, 3).Str;
    std::wcout << "\n";
    // std::wcout << data.ToJSON().Str;
    // std::wcout << "\n";
}

int main() noexcept {
    run();

    // std::getchar();

    return 1;
}

String read_file(const char fullpath[]) noexcept {
    String content;

    std::ifstream file(fullpath, std::ios::ate | std::ios::out);
    if (file.is_open()) {
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);
        UNumber u_size = (UNumber(size) + 1);

        char *_tmp = new char[u_size];

        file.read(_tmp, size);
        _tmp[(u_size - 1)] = L'\0';

        String::SetSize(&content, u_size);
        UNumber ln = 0;

        while (_tmp[ln] != L'\0') {
            content.Str[ln] = _tmp[ln];
            ln++;
        }

        delete[] _tmp;
        file.close();
    } else {
        std::wcout << "\n" << fullpath << L" does not exist!\n";
    }

    return content;
}

Tree get_tree() noexcept {
    Tree data     = Tree();
    data.HashBase = 97;

    data[L"var1"]  = L"\"1\"";
    data[L"PP"]    = L"gg";
    data[L"nu"]    = nullptr;
    data[L"bool"]  = false;
    data[L"bool2"] = true;
    data[L"&&"]    = 1000.0;

    data[L"var2"]       = L"2";
    data[L"var2"]       = L"11"; // Testing overriding.
    data[L"var3"]       = L"3";
    data[L"var4"]       = L"4";
    data[L"var5"]       = L"5";
    data[L"num34"]      = Array<String>().Add(L"3").Add(L"4");
    data[L"var_string"] = L"image";
    data[L"engine"]     = L"Qentem";
    data[L"abc1"]       = Array<String>().Add(L"B").Add(L"C").Add(L"D").Add(L"A");
    data[L"abc2"]       = Array<String>().Add(L"E").Add(L"F").Add(L"A");
    data[L"numbers"]    = Array<double>().Add(0).Add(1).Add(2).Add(3).Add(4).Add(5);
    data[L"empty"]      = L"";
    data[L"math"] = L"((2* (1 * 3)) + 1 - 4) + (((10 - 5) - 6 + ((1 + 1) + (1 + 1))) * (8 / 4 + 1)) - (1) - (-1) + 2";

    data[L"abc"] = Tree();

    data[L"abc"][L"B"] = L"b";

    data[L"abc"][L"@@"] = 100.0;
    // data[L"abc"][L"TT"]=false;
    data[L"abc"][L"A"]   = L"a";
    data[L"abc"][L"C"]   = L"c";
    data[L"abc"][L"D"]   = L"d";
    data[L"abc"][L"E"]   = Array<String>().Add(L"O").Add(L"K!");
    data[L"abc"][L"A-Z"] = L"ABCDEFGHIGKLMNOBQRST.....";

    data[L"multi"]          = Tree();
    data[L"multi"][L"arr1"] = data[L"abc"];
    data[L"multi"][L"arr2"] = Array<String>().Add(L"B").Add(L"C").Add(L"D").Add(L"A");
    data[L"multi"][L"C"]    = L"cool";

    return data;
}
