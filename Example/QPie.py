from ctypes import c_wchar_p, CDLL, c_bool

QLib_render = CDLL("./Build/QLib.so").renderTemplate_w
QLib_render.restype = c_wchar_p
QLib_render.argtypes = [c_wchar_p, c_wchar_p, c_bool]

file_tempale = open("./Test/temp.qtml", "r")
tempale_text = file_tempale.read()
file_tempale.close()

file_json = open("./Test/temp.json", "r")
json_text = file_json.read()
file_json.close()

enable_comments = True

# Randering
rendered = QLib_render(c_wchar_p(tempale_text),
                       c_wchar_p(json_text), enable_comments)

print(rendered)
