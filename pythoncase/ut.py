#!/usr/bin/env python3
# -*- coding: utf-8 -*-

def fact(n):
	if n < 1:
		raise ValueError('error para %d' % n)
	if n == 1:
		return 1
	return n * fact(n-1)

import unittest
class Testfact(unittest.TestCase):
	def test_1(self):
		a = fact(1)
		self.assertEqual(a, 1)

	def test_9(self):
		b = fact(9)
		c = fact(8)
		self.assertEqual(b, c * 9)

	def test_excp(self):
		with self.assertRaises(ValueError):
			fact(0)

if __name__ == '__main__':
	unittest.main()
