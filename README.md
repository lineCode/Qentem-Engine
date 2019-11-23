# Qentem Engine (v1.5.5)

- [Introduction](#introduction)
- [Why Qentem?](#why-qentem)
- [Built-in](#built-in)
- [Requirements](#requirements)
- [Example](#example)
  - [Document and JSON (C++)](#document-and-json-c)
  - [Template](#template)
    - [C++](#c)
    - [Python](#python)
    - [JavaScript & WebAssembly](#html-javascript--webassembly)
- [Compiling](#compiling)

## Introduction
Qentem Engine is an independent library that uses a fast algorithm for nest-matching. It can be used to match existing syntaxes or new ones, and with call-backs for post and per-parsing, It's posable to match almost any complex syntax. It is very efficient and has a small footprint on memory, and it's built using modern C++. It can be used to render complex templates that contains nested loop, nested if-else, inline if, math (+ * / - ^ %), logic (&& ||), and/or something simple: like replacing a text or splitting it. Also, it is capable of parsing JSON & XML/HTML.

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
### Document and JSON (C++):
```cpp
#include <Extension/Document.hpp>
using Qentem::Array;
using Qentem::Document;
using Qentem::String;

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
```
##### Output (Formatted):
```json
{
    "numbers": [
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        9
    ],
    "strings": {
        "a": "A",
        "b": "B",
        "c": "C",
        "d": "D"
    },
    "strings2": [
        "E",
        "F",
        "G",
        "H",
        "I"
    ],
    "bool": [
        true,
        true,
        true,
        true
    ]
}
```

### Template:

#### C++:
```cpp
#include <Extension/Template.hpp>
using Qentem::String;

// Based on the Document example

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

String rendered = Template::Render(content, &doc);

std::cout << "Template:\n" << rendered.Str << '\n';
```

##### Output:
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


        true

```

##### Note:
The complete example is located @ [Example/Example1.cpp](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Example/Example1.cpp). For more about template syntax, check out [Test/test.qtml](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Test/test.qtml).

#### Python:
```python
from ctypes import CDLL, c_char_p, c_bool
import json

q_render = CDLL("./Build/QLib.so").qentem_render_template
# Use QLib.dll on Windows.
q_render.restype = c_char_p
q_render.argtypes = [c_char_p, c_char_p, c_bool]

tempale = """
Students' list:
<loop set="major" key="_m_">
    Major: _m_
    <loop set="major[_m_]" key="_i_">
        Student's Name: {v:major[_m_][_i_][Name]}
        GPA: {v:major[_m_][_i_][GPA]}
        <if case="{v:major[_m_][_i_][GPA]} < 2.5"> Inform adviser!
        <elseif case="{v:major[_m_][_i_][GPA]} >= 3.5" /> President's List!
        <elseif case="{v:major[_m_][_i_][GPA]} >= 3.0" /> Dean's List!
        </if>
    </loop>
</loop>
"""

data = {
    'major': {
        'Computer Science': [
            {'Name': 'Oliver', 'GPA': '3.2'},
            {'Name': 'Jonah', 'GPA': '3.8'},
            {'Name': 'Ava', 'GPA': '2.8'}
        ],
        'Math': [
            {'Name': 'Maxim', 'GPA': '3.0'},
            {'Name': 'Cole', 'GPA': '2.5'},
            {'Name': 'Claire', 'GPA': '2.4'}
        ]
    }
}

print(q_render(tempale.encode('UTF-8'),
               json.dumps(data).encode('UTF-8'), False).decode(encoding='UTF-8'))
# Note: "False" means JSON without comments.
```
##### Output:
```txt
Students' list:

    Major: Computer Science

        Student's Name: Oliver
        GPA: 3.2
         Dean's List!


        Student's Name: Jonah
        GPA: 3.8
         President's List!


        Student's Name: Ava
        GPA: 2.8


    Major: Math

        Student's Name: Maxim
        GPA: 3.0
         Dean's List!


        Student's Name: Cole
        GPA: 2.5


        Student's Name: Claire
        GPA: 2.4
         Inform adviser!
```

##### Note:
For another example, check out [Example/Python/QPie1.py](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Example/Python/QPie1.py).

#### HTML, JavaScript & WebAssembly:
```html
<!DOCTYPE html>
<html>

<body>
    <div id="maindiv"></div>
    <script>
        var Module = {
            onRuntimeInitialized: function () {
                var data, template = `
                    <h2>Students' list:</h2>
                    <loop set="major" key="_m_">
                        <h3>Major: _m_</h3>
                        <ul>
                        <loop set="major[_m_]" key="_i_">
                            <li>
                                <span>Name: {v:major[_m_][_i_][Name]}</span>
                                <span>
                                    GPA: {v:major[_m_][_i_][GPA]}
                                    <if case="{v:major[_m_][_i_][GPA]} < 2.5"> (Inform adviser!)
                                    <elseif case="{v:major[_m_][_i_][GPA]} >= 3.5" /> (President's List!)
                                    <elseif case="{v:major[_m_][_i_][GPA]} >= 3.0" /> (Dean's List!)
                                    </if>
                                </span>
                            </li>
                        </loop>
                        </ul>
                    </loop>`;

                data = {
                    'major': {
                        'Computer Science': [
                            { 'Name': 'Oliver', 'GPA': '3.2' },
                            { 'Name': 'Jonah', 'GPA': '3.8' },
                            { 'Name': 'Ava', 'GPA': '2.8' }
                        ],
                        'Math': [
                            { 'Name': 'Maxim', 'GPA': '3.0' },
                            { 'Name': 'Cole', 'GPA': '2.5' },
                            { 'Name': 'Claire', 'GPA': '2.4' }
                        ]
                    }
                };

                document.getElementById("maindiv").innerHTML = Module.ccall(
                    'qentem_render_template',
                    'string',
                    ['string', 'string', 'bool'],
                    [template, JSON.stringify(data), false]
                );
            }
        };
    </script>
    <script src="JQen.js"></script>
</body>

</html>
```
Download JQen to run the example: [JQen.zip](https://github.com/HaniAmmar/Qentem-Engine/releases/download/v1.5.5/JQen.zip) or compile it using Emscripten (See QLib).

## Compiling

### The example:
```txt
c++ -I ./Source ./Example/Example1.cpp -o ./Test/QExample1.bin
```

### QLib (WebAssembly using Emscripten):
```txt
em++ -Os -I ./Source ./Example/QLib.cpp -s WASM=1 -s 'EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']' -o ./Example/JQen/JQen.js
```

### QLib (Template library):

#### Linux:
```txt
c++ -O3 -shared -fPIC -I ./Source ./Example/QLib.cpp -o ./Build/QLib.so
```

#### Windows:
Use QDLL.vcxproj (Visual Studio file).
