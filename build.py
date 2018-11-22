#!/usr/bin/env python3

import sys
import glob
import subprocess


def find_compiler():
    options = ("clang++", "g++")
    if sys.argv.count("--gcc") == 1:
        options = ("g++",)
    if sys.argv.count("--llvm") == 1:
        options = ("clang++",)

    for compiler in options:
        try:
            subprocess.check_output(("which", compiler))
            return compiler
        except subprocess.CalledProcessError:
            continue
    print("Unable to find a c++ compiler.\n")
    exit(1)


def pkg_config(args):
    return subprocess.check_output(args.split(' ')).decode().strip().split(' ')


if __name__ == "__main__":
    compiler = [find_compiler()]
    compiler_args = ['-ohail_eris', '-std=c++14']
    linker_args = pkg_config("pkg-config --static --libs glfw3")
    linker_args += pkg_config("pkg-config --static --libs glew")
    defines = {}

    debug_build = bool(sys.argv.count("--debug") or sys.argv.count("-d"))
    if debug_build:
        print("\x1b[3;30;47m{}\x1b[0m".format("Now with debugging!"))
        compiler_args += ['-O0', '-g']
        defines["DEBUG_BUILD"] = 1

    sources = glob.glob("**.cpp", recursive=True)

    build = []
    build += compiler
    build += compiler_args
    build += set(linker_args)
    build += ["-D{}={}".format(*kv) for kv in defines.items()]
    build += sources

    print(" ".join(build))
    subprocess.run(build, stdout=sys.stdout, stderr=sys.stdout)
