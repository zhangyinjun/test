#!/usr/bin/env python3
# -*- coding: utf-8 -*-

def func1(n):
	while n > 0:
		x = yield n
		print(x)
		n = n - 1
