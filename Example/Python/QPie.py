from ctypes import c_wchar_p, CDLL, c_bool

q_template = CDLL("./Build/QLib.so").qentem_render_template_w
q_template.restype = c_wchar_p
q_template.argtypes = [c_wchar_p, c_wchar_p, c_bool]

file_tempale = open("./Tests/test.qtml", "r")
tempale_text = file_tempale.read()
file_tempale.close()

file_json = open("./Tests/test.json", "r")
json_text = file_json.read()
file_json.close()

enable_comments = False

# Randering
rendered = q_template(c_wchar_p(tempale_text),
                      c_wchar_p(json_text), enable_comments)

print(rendered)
