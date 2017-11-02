#!/usr/bin/env python3
# -*- coding: utf-8 -*-

if __name__ == '__main__':
	name = input("please input your name: ")
	print("你好,", name)

	num = input("pleanse input a number: ")
	b = int(num)
	if b >= 0:
		print("positive number: %d" % b)
	else:
		print("negative number: %d" % b)

	L = ['chery', 'bart', 'jack']
	for x in L:
		print("hello,", x)

import math

def quadratic(a, b, c):
	x = (-b + math.sqrt(b*b - 4*a*c))/(2*a)
	y = (-b - math.sqrt(b*b - 4*a*c))/(2*a)
	return x,y

class Chain(object):
	def __init__(self, path=''):
		self._path = path

	#def __getattr__(self, attr):
	#	return lambda x:Chain('/%s/:%s' % (attr, x))
	def __getattr__(self, attr):
		return Chain('%s/%s' % (self._path, attr))

	def users(self, name):
		return Chain('/users/:%s' % name)

	def __str__(self):
		return self._path

	__repr__ = __str__

class Class_1(object):
	def __init__(self, name=''):
		self.__name = name

	def __getattr__(self, attr):
		return Class_1('%s/%s' % (self.__name, attr))

	def __call__(self, name):
		return Class_1('%s/:%s' % (self.__name, name))

	def __repr__(self):
		return self.__name
