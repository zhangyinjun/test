#!/usr/bin/env python3

def triangles():
	l = []
	while True:
		k = len(l) - 1
		while k > 0:
			l[k] = l[k] + l[k-1]
			k = k - 1
		l.append(1)
		yield l 
			
