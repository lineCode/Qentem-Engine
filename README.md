# Qentem Engine (v1.4.4)

## Introduction:
Qentem Engine is an independent library that uses a fast algorithm for nest-matching. It can be used to match existing syntaxes or new ones, and with call-backs for post and per-parsing, It's posable to match almost any complex syntax. It is very efficient and has a small footprint on memory, and it's built using modern C++. It can be used to render complex templates that contains nested loop, nested if-else, inline if, math (+ * / - ^ %), logic (&& ||), and/or something simple: like replacing a text or splitting it. Also, it is capable of doing JSON, XML/HTML.

## Why Qentem?:
Qentem is a port of a PHP script that was used to render HTML templates. It was based on regular expression, and beside being limited, it was slow. However, the limitation was the main reason for making Qentem.

The limitation comes from being dependent on regular expression, as any expert on Regex knows that it does not handle nest-matching very well, and sometimes, it does not do it at all. For instance, nesting “if” condition that has “else/elseif”. Regex can do one level, like:

```html
<if ...>...
<elseif ...>...
<else>...
</if>
```

However, the following is out of the question:

```html
 <if ...>…
    <if ...>…</if>...
<elseif ...>…
    <if ...>…<else>…</if>…
<else>…
    <if ...>…<elseif ...>…</if>…
</if>

...

 <if ...>…
    <if ...>…</if>...
</if>
```

Qentem can generate a template with that, and it can run on the client side, after compiling it to WebAssembly.

## Built-in:
The library - at the moment - has String class (with number conversion), Array, String Stream, Document (Tree/Map/HashTable), Template generator (HTML friendly syntax), Arithmetic & Logic Evaluator, and JSON parser (with C style comments). Also, XML/HTML parser.

## Requirements:
C++ compiler (11 and above).

## Template Demo:
HTML, JavaScript and WebAssembly (requires web server to run; local or remote): [JQen.zip](https://github.com/HaniAmmar/Qentem-Engine/releases/download/v1.4.3/JQen.zip)

## Example:
### Document and JSON:
```cpp
#include <Extension/Document.hpp>
using Qentem::Array;
using Qentem::Document;
using Qentem::String;

Document numbers = L"[null,1,2,3,4]"; // New document from JSON text

numbers += Document::FromJSON(L"[5,6]");  // Extanding JSON
numbers += Array<double>().Add(7).Add(8); // Extanding win ordered array

Document doc;              // New document
doc[L"numbers"] = numbers; // assigning ordered numbers
doc[L"numbers"] += 9;      // Add
doc[L"numbers"][0] = 0;    // Override

doc += Document::FromJSON(L"{\"strings\": { }}"); // Expanding document with unordered array

doc[L"strings"][L"a"] = nullptr; // null
doc[L"strings"][L"a"] = L"A";    // Override
doc[L"strings"][L"b"] = true;    // Add & Set
doc[L"strings"][L"b"] = false;   // Override
doc[L"strings"][1]    = L"B";    // Override

// Importing JSON with comments
doc[L"strings"] += Document::FromJSON(L"{\"c\": \"C\", \"d\": \"D\"  /* \"e\": \"E\" */}", true);

doc += Document::FromJSON(L"{\"strings2\": [\"E\", \"F\"]}"); // Ordered strings
doc[L"strings2"] += L"G";
doc[L"strings2"] += Array<String>().Add(L"H").Add(L"I");

String JSON = doc.ToJSON(); // Exporting document
std::wcout << L"JSON:\n" << JSON.Str << L"\n\n";
```
#### Output:
```json
{
    "numbers": [0,1,2,3,4,5,6,7,8,9],
    "strings": {"a": "A","b": "B","c": "C","d": "D"},
    "strings2": ["E","F","G","H","I"]
}
```

### Template:
```cpp
#include <Extension/Template.hpp>
using Qentem::String;
// Based on the Document example
String content = L"<loop set=\"strings\" value=\"s_value\" key=\"s_key\">s_key: s_value\n</loop>--\n";
content += L"<loop set=\"strings2\" value=\"s_value\" key=\"s_key\">s_key: s_value\n</loop>";
String rendered = Template::Render(content, &doc);
std::wcout << L"Template:\n" << rendered.Str << L'\n';
```
#### Output:
```txt
a: A
b: B
c: C
d: D
--
0: E
1: F
2: G
3: H
4: I
```

### Note:
The complete example is located @ [Example/Example1.cpp](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Example/Example1.cpp). For more about template syntax, see [Test/test.qtml](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Test/test.qtml).


## Compiling:

### The example:
```txt
c++ -I ./Source ./Example/Example1.cpp -o ./Test/QExample1.bin
```

### QLib (Template library):
```txt
c++ -O3 -shared -fPIC -I ./Source ./Example/QLib.cpp -o ./Build/QLib.so
```

### QLib (WebAssembly using Emscripten):
```txt
em++ -Os -Wall -std=c++11 -I ./Source ./Example/QLib.cpp -s WASM=1 -s 'EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']' -o ./Example/JQen/JQen.js
```

## Test:
This library includes over 370 tests; to insure that it's stable and functional. to compile the tests:
```txt
c++ -I ./Source ./Test/Test.cpp -o ./Test/QTest.bin
```
