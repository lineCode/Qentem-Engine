# Qentem Engine (v1.5.1)

## Introduction
Qentem Engine is an independent library that uses a fast algorithm for nest-matching. It can be used to match existing syntaxes or new ones, and with call-backs for post and per-parsing, It's posable to match almost any complex syntax. It is very efficient and has a small footprint on memory, and it's built using modern C++. It can be used to render complex templates that contains nested loop, nested if-else, inline if, math (+ * / - ^ %), logic (&& ||), and/or something simple: like replacing a text or splitting it. Also, it is capable of doing JSON, XML/HTML.

## Why Qentem?
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

Qentem can generate a template with that, and it's usable with JavaScript (Node.js/Web browser), after compiling it to WebAssembly.

## Built-in
The library - at the moment - has String class (with number conversion), Array, String Stream, Document (Tree/Map/HashTable), Template generator (HTML friendly syntax), Arithmetic & Logic Evaluator, and JSON parser (with C style comments). Also, XML/HTML parser.

## Requirements
C++ compiler (11 and above).

## Example
### Document and JSON
```cpp
#include <Extension/Document.hpp>
using Qentem::Array;
using Qentem::Document;
using Qentem::String;

Document numbers = "[null,1,2,3,4]"; // New document from JSON text

numbers += Document::FromJSON("[5,6]");  // Extanding JSON
numbers += Array<double>().Add(7).Add(8); // Extanding win ordered array

Document doc;              // New document
doc["numbers"] = numbers; // assigning ordered numbers
doc["numbers"] += 9;      // Add
doc["numbers"][0] = 0;    // Override

doc += Document::FromJSON("{\"strings\": { }}"); // Expanding document with unordered array

doc["strings"]["a"] = nullptr; // null
doc["strings"]["a"] = "A";    // Override
doc["strings"]["b"] = true;    // Add & Set
doc["strings"]["b"] = false;   // Override
doc["strings"][1]    = "B";    // Override

// Importing JSON with comments
doc["strings"] += Document::FromJSON("{\"c\": \"C\", \"d\": \"D\"  /* \"e\": \"E\" */}", true);

doc += Document::FromJSON("{\"strings2\": [\"E\", \"F\"]}"); // Ordered strings
doc["strings2"] += "G";
doc["strings2"] += Array<String>().Add("H").Add("I");

String JSON = doc.ToJSON(); // Exporting document
std::cout << "JSON:\n" << JSON.Str << "\n\n";
```
the output is somthing close to:
```json
{
    "numbers": [0,1,2,3,4,5,6,7,8,9],
    "strings": {"a": "A","b": "B","c": "C","d": "D"},
    "strings2": ["E","F","G","H","I"]
}
```

### Template

#### C++
```cpp
#include <Extension/Template.hpp>
using Qentem::String;
// Based on the Document example
String content = "<loop set=\"strings\" value=\"s_value\" key=\"s_key\">s_key: s_value\n</loop>--\n";
content += "<loop set=\"strings2\" value=\"s_value\" key=\"s_key\">s_key: s_value\n</loop>";
String rendered = Template::Render(content, &doc);
std::cout << "Template:\n" << rendered.Str << '\n';
```

##### Note
The complete example is located @ [Example/Example1.cpp](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Example/Example1.cpp). For more about template syntax, check out [Test/test.qtml](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Test/test.qtml).

#### Python
```python
from ctypes import CDLL, c_char_p, c_bool

q_render = CDLL("./Build/QLib.so").qentem_render_template
# Use QLib.dll on Windows.
q_render.restype = c_char_p
q_render.argtypes = [c_char_p, c_char_p, c_bool]

tempale = """
<loop set="numbers" value="this_number">
<if case="(this_number % 2) == 1">this_number is an odd number.</if></loop>

{v:eq} = {math: {v:eq}}

(0.1 + 0.2) is {math:   0.1   +   0.2  }

{iif case="{v:qen} = Qentem" true="{v:qen}"} Engine

{iif case="{v:numbers[0]} = v:numbers[4]" false="it's not {v:numbers[4]}!"}
"""

json_text = '{"numbers":[1,2,3,4,5,6,7,8], "eq": "((1+2)^3)/2", "qen": "Qentem"}'

print(q_render(tempale.encode('UTF-8'),
               json_text.encode('UTF-8'), False).decode(encoding='UTF-8'))
# Note: "False" means JSON without comments.
```

#### HTML, JavaScript and WebAssembly
[JQen.zip](https://github.com/HaniAmmar/Qentem-Engine/releases/download/v1.5.0/JQen.zip)

## Compiling

### The example
```txt
c++ -I ./Source ./Example/Example1.cpp -o ./Test/QExample1.bin
```

### QLib (Template library)
#### Linux
```txt
c++ -O3 -shared -fPIC -I ./Source ./Example/QLib.cpp -o ./Build/QLib.so
```

#### Windows
Run QDLL.vcxproj (Visual Studio file).

### QLib (WebAssembly using Emscripten):
```txt
em++ -Os -I ./Source ./Example/QLib.cpp -s WASM=1 -s 'EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']' -o ./Example/JQen/JQen.js
```
