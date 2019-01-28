
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
#include <fstream>
#include <iostream>

using Qentem::Array;
using Qentem::String;
using Qentem::Template;
using Qentem::Tree;

Tree   get_data() noexcept;
String get_template() noexcept;

int main() {
    String content = get_template();
    Tree   data    = get_data();

    Template _template = Template();

    std::wcout << _template.Render(content, &data).Str;

    // std::getchar();

    return 1;
}

String get_template() noexcept {
    String temp;

    // Reading a template file.
    std::ifstream file("./Test/temp.qtml", std::ios::ate | std::ios::out);
    if (file.is_open()) {
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);

        auto _tmp = new char[(int(size) + 1)];

        file.read(_tmp, size);

        UNumber ln = 0;
        String::SetSize(&temp, size);
        while (_tmp[ln] != L'\0') {
            temp += _tmp[ln++];
        }

        delete[] _tmp;
        file.close();
    } else {
        std::wcout << "\ntemp.qtml does not exist!\n";
        std::exit(EXIT_FAILURE);
    }

    return temp;
}

Tree get_data() noexcept {
    Tree data     = Tree();
    data.HashBase = 97;
    // Feature: Build json parser.
    data.Set(L"var1", L"1");
    data.Set(L"var2", L"2");
    data.Set(L"var2", L"11"); // Testing overriding.
    data.Set(L"var3", L"3");
    data.Set(L"var4", L"4");
    data.Set(L"var5", L"5");
    data.Set(L"num34", Array<String>().Add(L"3").Add(L"4"));
    data.Set(L"var_string", L"image");
    data.Set(L"engine", L"Qentem");
    data.Set(L"abc1", Array<String>().Add(L"B").Add(L"C").Add(L"D").Add(L"A"));
    data.Set(L"abc2", Array<String>().Add(L"E").Add(L"F").Add(L"A"));
    //  data.Set(L"numbers", {"0", L"1", L"2", L"3", L"4"}); when initializer_list is enabled
    data.Set(L"numbers", Array<String>().Add(L"0").Add(L"1").Add(L"2").Add(L"3").Add(L"4"));
    data.Set(L"empty", L"");
    data.Set(L"math", L"((2* (1 * 3)) + 1 - 4) + (((10 - 5) - 6 + ((1 + 1) + (1 + 1))) * (8 / 4 + 1)) - (1) - (-1) + 2");

    // The equivlent of; abc=>{"B"=>"b", L"A"=>"a", ...}
    Tree group1 = Tree();
    group1.Set(L"B", L"b");
    group1.Set(L"A", L"a");
    group1.Set(L"C", L"c");
    group1.Set(L"D", L"d");
    group1.Set(L"E", Array<String>().Add(L"O").Add(L"K!"));
    group1.Set(L"A-Z", L"ABCDEFGHIGKLMNOBQRST.....");

    data.Set(L"abc", group1);

    Tree group2 = Tree();
    group2.Set(L"arr1", group1);
    group2.Set(L"arr2", Array<String>().Add(L"B").Add(L"C").Add(L"D").Add(L"A"));
    group2.Set(L"C", L"cool");

    data.Set(L"multi", group2);

    return data;
}
