from ctypes import c_wchar_p, CDLL, c_bool

q_render = CDLL("./Build/QLib.so").qentem_render_template_w
# Use QLib.dll for Windows.
q_render.restype = c_wchar_p
q_render.argtypes = [c_wchar_p, c_wchar_p, c_bool]

json_text = '{"numbers":[1,2,3,4,5,6,7,8], "eq": "((1+2)^3)/2", "qen": "Qentem"}'

tempale = """
<loop set="numbers" value="this_number">
<if case="(this_number % 2) == 1">this_number is an odd number.</if></loop>

{v:eq} = {math: {v:eq}}

(0.1 + 0.2) is {math:   0.1   +   0.2  }

{iif case="{v:qen} = Qentem" true="{v:qen}"} Engine

{iif case="{v:numbers[0]} = v:numbers[4]" false="it's not {v:numbers[4]}!"}
"""

print(q_render(tempale, json_text, False))
# Note: "False" means JSON without comments.
