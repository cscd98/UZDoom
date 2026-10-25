"""Extract standalone menu domains; the runtime supplies current enum values."""
import json
import pathlib
import re
import sys
text = pathlib.Path(sys.argv[1]).read_text()
token = r'"?([\w.]+)"?'
rows = []
for match in re.finditer(r'^\s*Option\s+"[^"\n]*"\s*,\s*' + token + r'\s*,\s*' + token, text, re.M):
    name, group = match.groups()
    rows.append('{%s, %s, 0, 0, 0},' % (json.dumps(name), json.dumps(group)))
for match in re.finditer(r'^\s*Slider\s+"[^"\n]*"\s*,\s*' + token + r'\s*,\s*([-\d.]+)\s*,\s*([-\d.]+)\s*,\s*([-\d.]+)', text, re.M):
    name, low, high, step = match.groups()
    rows.append('{%s, nullptr, %s, %s, %s},' % (json.dumps(name), low, high, step))
pathlib.Path(sys.argv[2]).write_text('// Generated from standalone menudef.txt.\n' + '\n'.join(rows))
