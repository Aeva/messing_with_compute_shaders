
import re
import os.path


INCLUDE_REGEX = re.compile(r'^\s*#include\s+\"([^\"]+)\"\s*$', re.I)
ONCE_REGEX = re.compile(r'^\s*#pragma once\s*$', re.I)


def apply_includes(path, skip):
    abspath = os.path.abspath(path)
    new_file = ""
    with open(path, "r") as infile:
        for counter, line in enumerate(infile):
            once = ONCE_REGEX.match(line)
            if once:
                skip.append(abspath)
                new_file += "\n"
                continue
                        
            match = INCLUDE_REGEX.match(line)
            if match:
                file_name = match.groups()[0]
                new_path = os.path.abspath(file_name)
                if skip.count(new_path):
                    new_file += "\n"
                    continue

                new_file += "// BEGIN INCLUDED FILE " + file_name + "\n"
                new_file += apply_includes(new_path, skip)
                new_file += "// END INCLUDED FILE " + file_name + "\n"
                continue

            new_file += line

    return ''.join(new_file)


def process_file(path):
    new_path = path + ".built"
    new_data = apply_includes(path, [])
    with open(new_path, "w") as outfile:
        outfile.write(new_data)
