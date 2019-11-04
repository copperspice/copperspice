#!/usr/bin/env python3
""" pype - redirect output to a file and to std.err (making it unbuffered / live-streaming)
"""

import traceback, sys, os, time
from datetime import datetime
from util import *
from subprocess import Popen, PIPE, STDOUT

filter_msvc = False
summary_msvc = False
filter_gcc = False
summary_gcc = False
summary = []

def show_usage():
    eprint ("pype - redirect stdout+stderrr of <cmd> to a file and to stderr")
    eprint ("usage: pipe <filename> -- <cmd>")

def format_delta(delta):
    sec = delta.total_seconds()
    hours = int(sec /60 /60)
    sec = sec - (60*60*hours)
    minutes = int(sec /60)
    seconds = sec - (60*minutes)
    return str.format("{0:d}h {1:d}m {2:.3f}s", hours, minutes, seconds)


def process(line):
    global summary

    is_error = False
    if summary_msvc or filter_msvc:
        is_error = b": error " in line

    if summary_gcc or filter_gcc:
        is_error = b": error " in line

    if summary_msvc or summary_gcc:
        if is_error:
            summary += [line]

    output_message = True
    if filter_msvc:
        if line.startswith(b" "):
            output_message = False
        if not is_error and b": warning" in line:
            output_message = False
        if b": note" in line:
            output_message = False

    if filter_gcc:
        if b"[-W" in line:
            output_message = False

    if output_message:
        # write to unbuffered stderr output to get a responsive line-by-line build log
        sys.stderr.buffer.write(line)

def main():
    global filter_msvc
    global summary_msvc
    global filter_gcc
    global summary_gcc

    if len(sys.argv) < 4:
        show_usage()
        sys.exit(1)
    position = 0
    for arg in sys.argv:
        position = position + 1
        if arg == "--":
            break
        if "filter-msvc-warnings" in arg:
            filter_msvc = True
        if "add-msvc-summary" in arg:
            summary_msvc = True
        if "filter-gcc-warnings" in arg:
            filter_gcc = True
        if "add-gcc-summary" in arg:
            summary_gcc = True

    cmd = " ".join(sys.argv[position:])
    lastline = ""

    starttime = datetime.now()

    # combine sub-process' stdout+stderr into one stream using `stdout=PIPE, stderr=STDOUT`
    # see https://docs.python.org/3/library/subprocess.html
    with open(sys.argv[1], 'wb') as outputfile:
        sub_process = Popen(cmd, stdout=PIPE, stderr=STDOUT, shell=True)
        for line in sub_process.stdout:
            try:
                outputfile.write(line)
                process(line)
                lastline = line
            except Exception as ex:
                eprint("PYPE", type(ex).__name__ + ":", ex)
                eprint("## last: ", lastline)
                eprint("## now : ", line)
                sys.exit(1)
    sub_process.wait()
    rc = sub_process.returncode
    elapsedTime = datetime.now() - starttime

    if summary_gcc or summary_msvc:
        eprint("-- Build summary --")
        for line in summary:
            eprint(line.strip().decode("utf-8"))

    sys.stderr.write("PYPE subprocess exited with code " + str(rc) + " after " + format_delta(elapsedTime) + ".\n")
    sys.exit(rc)    # forward return code

if __name__ == "__main__":
    try:
        main()
    except SystemExit:
        raise
    except:
        info = traceback.format_exc()
        eprint(info)
        show_usage()
        sys.exit(1)