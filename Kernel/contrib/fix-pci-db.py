#!/usr/bin/python2
import re
import string

f = open("include/Alentours/PCIDB.h", "r")
d = f.read()
d = d.replace("char *", "const char *")
d = re.sub(r'http://pcidatabase\.com/update_device\.php[^"]*"', '"', d)
f.close()

lines = d.split("\n")
newlines = []
in_ventable = False
in_devtable = False

for line in lines:
    if line.startswith('PCI_DEVTABLE'):
        in_devtable = True
        print "[i] In devtable"
        newlines.append(line)
        continue
    if in_devtable and line.startswith("{"):
        newlines.append(line)
        continue
    if in_devtable and line.startswith("}"):
        newlines.append(line)
        in_devtable = False
        continue

    if line.startswith('PCI_VENTABLE'):
        in_ventable = True
        print "[i] In ventable"
        newlines.append(line)
        continue
    if in_ventable and line.startswith("{"):
        newlines.append(line)
        continue
    if in_ventable and line.startswith("}"):
        in_ventable = False
        newlines.append(line)
        continue

    if in_devtable:
        if not re.match('\t' + r'\{ 0x([a-fA-F0-9]){4}, 0x([a-fA-F0-9]){4}, "[^"\\]*", "[^"\\]*".+', line) or line.find("??") >= 0:
            print "[i] Removing invalid line %s from PCIDB.h..." % line
        else:
            newlines.append(filter(lambda x: x in string.printable, line))
    elif in_ventable:
        if not re.match('\t' + r'{ 0x([a-fA-F0-9]){4}, "[^"\\]*", "[^"\\]*".+', line):
            print "[i] Removing invalid line %s from PCIDB.h..." % line
        else:
            newlines.append(filter(lambda x: x in string.printable, line))
    else:
        newlines.append(filter(lambda x: x in string.printable, line))

f = open("include/Alentours/PCIDB.h", "w")
f.write("\n".join(newlines))
f.close()
