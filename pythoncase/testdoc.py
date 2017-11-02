#!/usr/bin/env python3
# -*- coding: utf-8 -*-

def fact(n):
	'''
	>>> fact(1)
	1
	>>> fact(9)==fact(8)*9
	True
	>>> fact(-2)
	Traceback (most recent call last):
		...
	ValueError: error para -2
	'''
	if n < 1:
		raise ValueError('error para %d' % n)
	if n == 1:
		return 1
	return n * fact(n-1)

if __name__ == '__main__':
	import doctest
	doctest.testmod()

