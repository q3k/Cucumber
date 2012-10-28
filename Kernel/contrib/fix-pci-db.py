#!/usr/bin/python2
import re

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
	if line.startswith('	{ 0x165C, 0x0002'):
		in_devtable = True
	if in_devtable and line.startswith("}"):
		in_devtable = False
	
	if in_devtable:
		if not re.match(r'	{ 0x([a-fA-F0-9]){4}, 0x([a-fA-F0-9]){4}, "[^"\\]*", "[^"\\]*".+', line) or line.find("??") >= 0:
			print "[i] Removing invalid line %s from PCIDB.h..." % line
		else:
			newlines.append(line)
	else:
		newlines.append(line)

f = open("include/Alentours/PCIDB.h", "w")
f.write("\n".join(newlines))
f.close()



