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
