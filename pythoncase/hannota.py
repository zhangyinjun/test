#!/usr/bin/env python3

def move(n, a='a', b='b', c='c'):
	if n==1:
		print("%s-->%s" % (a,c))
		return

	move(n-1, a, c, b)
	print("%s-->%s" % (a,c))
	move(n-1, b, a, c)	
	return
