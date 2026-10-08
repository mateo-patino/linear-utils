import json
import argparse
import os

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Create database of compile commands")

    # Name of .json file to write commands to
    parser.add_argument(
        "--filename",
        required=True,
        type=str,
        help="File name where compile commands will be saved"
    )

    # Current directory (should be $(CURDIR))
    parser.add_argument(
        "--curdir",
        required=True,
        type=str,
        help="Current working directory"
    )

    # Compile command (should be $(COMPILE.c))
    parser.add_argument(
        "--compile-cmd",
        required=True,
        type=str,
        help="Command for compiling .c files"
    )

    # List of source files (app and test files)
    parser.add_argument(
        "--srcs",
        required=True,
        nargs='+',
        help="List of source file paths to save compile commands for"
    )

    # Tell the program to append to JSON file. This flag lets you call this program multiple times and append on each call.
    parser.add_argument(
        "--append",
        action="store_true",
        help="Append compile entries to the file provided"
    )


    args = parser.parse_args()

    file_name = args.filename
    data = []

    # If append flag is up, read any existing data from the file, append to it, and rewrite it
    if args.append:
        with open(file_name, "r") as existing_file:
            data = json.load(existing_file)

    # Append new entries
    for src_name in args.srcs:
        data.append({   
            "directory": args.curdir,
            "command": args.compile_cmd + f" -c {src_name} -o _",
            "file": os.path.join(args.curdir, src_name)
        })

    # Dump list of dictionaries in target file
    with open(file_name, "w") as file:
        json.dump(data, file, indent=4)

