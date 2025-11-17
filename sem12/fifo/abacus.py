#!/usr/bin/env python3

import sys

while True:

    try:

        str1 = sys.stdin.readline().strip()
        str2 = sys.stdin.readline().strip()

        mul1 = int(str1)
        mul2 = int(str2)

        product = mul1 * mul2

        str3 = str(product)

        sys.stdout.write(str3 + "\n")
        sys.stdout.flush()

    except Exception as e:
    
        sys.stdout.write(f"Error: \"{e}\"!\n")
        sys.stdout.flush()

