#!/usr/bin/env python3

from ctypes import *

ll = CDLL("../test.so")

a = ll.testcase3()
print("res:%d" % a)
v=c_int.in_dll(ll, "g_test") #get global variable g_test
print("g_test:%d" % v.value)
v.value=100 #modify global variable g_test
ll.testcase3()
