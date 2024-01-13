#!/usr/bin/python3
import argparse

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.usage = "./script.py [-h] '6de60408 ffe60408 ...'"
    parser.add_argument("hex_str", help="hex string to convert from little endian to big endian, and print to string value.")
    return (parser.parse_args())  

def big_to_little_endian(hex_str: str):
    flag = ""
    for val in hex_str.split():
        converted_value = bytes.fromhex(val)[::-1]
        flag += converted_value.decode("utf-8")
    return flag

if __name__ == "__main__":
    try:
        args = parse_arguments()
        flag = big_to_little_endian(args.hex_str)
        print(f"Flag: {flag}")
    except (ValueError, IndexError) as e:
        print(f"./script.py: {e}")