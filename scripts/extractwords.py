#!/usr/bin/python3
# Extracts words from a file.

import sys

splist = [
"\f(CW",
"\fI",
"\fP",
":",
";",
")",
"(",
"]",
"[",
"{",
"}",
"}",
"/",
"\\",
"*",
"'",
'"',
'<',
'>',
"=",
"+",
"-",
"#",
"%",
",",
"!",
"."]

def okfirst(myc):
    if myc <= 'z' and myc >= 'a':
        return True
    if myc <= 'Z' and myc >= 'A':
        return True
    if myc == '_':
        return True
    return False

def seemsaname(wd):
    if len(wd) < 1:
        return False
    c = wd[0]
    if not okfirst(c):
        return False
    # Maybe check length, say 1 character
    # long things are not words?
    return True

def rmspecial(line):
    for s in splist:
       if line.find(s) == -1:
           continue
       line = line.replace(s," ")
    return line

def myreadin(name):
    try:
        file = open(name, "r")
    except IOError:
        print (" File could not be opened:", name)
        sys.exit(1)
    while 1:
        try:
            rec = file.readline()
        except:
            break
        if len(rec) < 1:
            break
        line = rec.strip()
        if len(line) < 1:
            # Ignore empty lines
            continue
        line = rmspecial(line)
        wds = line.split()
        for w in wds:
           if seemsaname(w):
               print(w)

if __name__ == '__main__':

    if len(sys.argv) < 2:
        print("File name required")
        sys.exit(1)
    for i in range(1,len(sys.argv)):
        myreadin(sys.argv[i])
