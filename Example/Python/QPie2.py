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
