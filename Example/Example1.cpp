
// #include <Extension/Document.hpp>
#include <Extension/Template.hpp>
#include <iostream>

using Qentem::Array;
using Qentem::Document;
using Qentem::String;
using Qentem::Template;
using Qentem::UNumber;

int main() {
    // Document and JSON
    Document numbers = L"[1,1,2,3,4]"; // New document from a JSON text

    numbers += L"[5,6]";                      // Extanding JSON
    numbers += Array<double>().Add(7).Add(8); // Extanding win an ordered array

    Document doc;             // New document
    doc["numbers"] = numbers; // assigning an ordered numbers
    doc["numbers"] += 9;
    doc["numbers"][0] = 0;

    doc += L"{\"strings\": { }}"; // Expanding the document with an unordered array
    doc["strings"][L"a"] = L"A";
    doc["strings"][L"b"] = L"O";
    doc["strings"][1]    = L"B"; // Override

    // Importing JSON with comments
    doc["strings"] += Document::FromJSON(L"{\"c\": \"C\", \"d\": \"D\"  /* \"e\": \"E\" */}", true);

    doc += L"{\"strings2\": [\"E\", \"F\"]}"; // Ordered strings
    doc["strings2"] += L"G";
    doc["strings2"] += Array<String>().Add(L"H").Add(L"I");

    String JSON = doc.ToJSON(); // Exporting the document
    std::wcout << L"JSON:\n" << JSON.Str << L"\n\n";

    // Output:
    // {
    //      "numbers": [0,1,2,3,4,5,6,7,8,9],
    //      "strings": {"a": "A","b": "B","c": "C","d": "D"},
    //      "strings2": ["E","F","G","H","I"]
    // }

    // Template
    String content = L"<loop set=\"strings\" value=\"s_value\" key=\"s_key\">s_key: s_value\n</loop>--\n";
    content += L"<loop set=\"strings2\" value=\"s_value\" key=\"s_key\">s_key: s_value\n</loop>";
    String rendered = Template::Render(content, &doc);

    std::wcout << L"Template:\n" << rendered.Str << L'\n';
}
