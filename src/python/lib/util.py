#!/usr/bin/env python

import os
import errno

def int2path(num, digits = 1):
    shift_digits = digits * 5
    mask = (1 << shift_digits) - 1
    digit_char = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz-_"

    ret = []
    digits_range = range(digits)
    while True:
        ret.append(os.path.sep)

        tmp_digits = num & mask;
        for i in digits_range:
            ret.append( digit_char[ tmp_digits & 31 ] )
            tmp_digits >>= 5
        num >>= shift_digits
        if num <= 0:
            break
    if ret:
        return "".join(reversed(ret))
    return ""

def makedir(path):
    try:
        os.makedirs(path)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise

if __name__ == "__main__":
    print int2path(0)
    print int2path(1)
    print int2path(0, 2)
    print int2path(1000)
