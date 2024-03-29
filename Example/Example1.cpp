// #include <Extension/Document.hpp>
#include <Extension/Template.hpp>
#include <iostream>

using Qentem::Array;
using Qentem::Document;
using Qentem::String;

int main() {
    Document numbers = "[null,1,2,3,4]"; // New document from JSON text

    numbers += Document::FromJSON("[5,6]");   // Extanding JSON
    numbers += Array<double>().Add(7).Add(8); // Extanding win ordered array

    Document doc;             // New document
    doc["numbers"] = numbers; // Assigning ordered numbers
    doc["numbers"] += 9;      // Add
    doc["numbers"][0] = 0;    // Override

    doc += Document::FromJSON(R"({"strings": { }})"); // Expanding document with unordered array

    doc["strings"]["a"] = nullptr; // null
    doc["strings"]["a"] = "A";     // Override
    doc["strings"]["b"] = true;    // Add
    doc["strings"]["b"] = false;   // Override
    doc["strings"][1]   = "B";     // Override

    // Importing JSON with comments
    doc["strings"] += Document::FromJSON(R"({"c": "C", "d": "D"  /* "e": "E" */})", true);

    doc += Document::FromJSON(R"({"strings2": ["E", "F"]})"); // Ordered strings
    doc["strings2"] += "G";
    doc["strings2"] += Array<String>().Add("H").Add("I");

    Document bool_list;
    bool_list[3] = true; // Will fill the gaps with the value "true".

    doc["bool"] = static_cast<Document &&>(bool_list); // Moving

    String JSON = doc.ToJSON();
    std::cout << "JSON:\n" << JSON.Str << "\n\n";

    // Output should be something like:
    // {
    //     "numbers":[0,1,2,3,4,5,6,7,8,9],
    //     "strings":{"a":"A","b":"B","c":"C","d":"D"},
    //     "strings2":["E","F","G","H","I"],
    //     "bool":[true,true,true,true]
    // }

    // Template
    String content = R"(
        <loop set="strings" value="s_value" key="s_key">
        s_key: s_value
        </loop>
         --
        <loop set="strings2" value="s_value" key="s_key">
        s_key: s_value
        </loop>

        {v:bool[2]}
    )";

    String rendered = Qentem::Template::Render(content, &doc);

    std::cout << "Template:\n" << rendered.Str << '\n';
}
