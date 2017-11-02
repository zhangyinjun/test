#!/usr/bin/env python3
# -*- coding: utf-8 -*-

' script for parsing tpf file '

__author__ = 'Yinjun Zhang'

import getopt, sys
from enum import Enum

def usage():
	print('#--------------------------------------------')
	print('#   		 usage')
	print('#--------------------------------------------')
	print('options	default	description')
	print('-h	 off	display this help infomation')
	print('-i <s>	 off	<s>: name of tpf file to parse')
	print('-o <s>	 off	<s>: name of output file')
	print('#--------------------------------------------')
	print('#   		 example')
	print('#--------------------------------------------')
	print('%s -h' % sys.argv[0])
	print('%s -i rtl_out.0.bin.tpf -o parse_res' % sys.argv[0])
	print('\n')

ldcmd = {0:'WrHash', 1:'WrBS', 2:'RdToBuffer', 3:'WrCombine', 4:'RdReg', 5:'WrReg', 6:'WrLpm', 7:'RdLpm'}

def getldcmd(bitstr):
	if bitstr[3::-1] == '1000':		#bit[3:0]
		return 0
	elif bitstr[3::-1] == '1001':		#bit[3:0]
		return 1
	elif bitstr[3::-1] == '1010':		#bit[3:0]
		return 2
	elif bitstr[3::-1] == '1011':		#bit[3:0]
		if bitstr[10] == '0':
			return 7
		elif bitstr[10] == '1':
			return 6
	else:
		if bitstr[8:6:-1] == '01':	#bit[8:7]
			return 4
		elif bitstr[8:6:-1] == '00':	#bit[8:7]
			return 3
		elif bitstr[8:6:-1] == '10':	#bit[8:7]
			return 5

	return -1


def wrcombine_parse(outstr, bitstr):
	oct = int(bitstr[6:3:-1], 2) 	#bit[6:4]
	mod = int(bitstr[2::-1], 2)	#bit[2:0]
	comm_msb = int(bitstr[24:16:-1], 2)	#bit[24:17]
	prio_we = int(bitstr[9], 2)		#bit[9]
	prio_lsb = int(bitstr[16:9:-1], 2)	#bit[16:10]
	prio_val = int(bitstr[44:24:-1], 2)	#bit[44:25]
	hash_we = int(bitstr[45], 2)		#bit[45]
	hash_addr = int(bitstr[58:45:-2], 2)	#bit[58:46]
	hash_val = int(bitstr[70:58:-1], 2)	#bit[70:59]
	xc_we = int(bitstr[71], 2)		#bit[71]
	xc_addr = int(bitstr[83:71:-1], 2)	#bit[83:72]
	xc_val = int(bitstr[104:83:-1], 2)	#bit[104:84]
	bin_we = int(bitstr[105], 2)		#bit[105]
	bin_val = int(bitstr[145:105:-1], 2)	#bit[145:106]
	bin_pos = int(bitstr[152:145:-1], 2)	#bit[152:146]
	bin_lsb = int(bitstr[154:152:-1], 2)	#bit[154:153]
	outstr = outstr + ('oct=%d mod=%d' % (oct, mod))
	if prio_we:
		outstr = outstr + (' prioAddr=%X prioData=%X' % ((comm_msb<<7)|prio_lsb, prio_val))
	if hash_we:
		outstr = outstr + (' hashAddr=%X hashData=%X' % (hash_addr, hash_val))
	if xc_we:
		outstr = outstr + (' xcAddr=%X xcData=%X' % (xc_addr, xc_val))
	if bin_we:
		outstr = outstr + (' binAddr=%X binPos=%X binData=%010X' % ((comm_msb<<2)|bin_lsb, bin_pos, bin_val))
	return outstr


def rdreg_parse(outstr, bitstr):
	oct = int(bitstr[6:3:-1], 2) 	#bit[6:4]
	mod = int(bitstr[2::-1], 2)	#bit[2:0]
	isel = int(bitstr[14:8:-1], 2)	#bit[14:9]
	addr = int(bitstr[29:14:-1], 2)	#bit[29:15]
	outstr = outstr + ('oct=%d mod=%d isel=%d addr=%X' % (oct, mod, isel, addr))
	return outstr


def wrreg_parse(outstr, bitstr):
	oct = int(bitstr[6:3:-1], 2) 		#bit[6:4]
	mod = int(bitstr[2::-1], 2)		#bit[2:0]
	isel = int(bitstr[14:8:-1], 2)		#bit[14:9]
	addr = int(bitstr[29:14:-1], 2)		#bit[29:15]
	data = int(bitstr[158:29:-1], 2)	#bit[158:30]
	outstr = outstr + ('oct=%d mod=%d isel=%d addr=%X data=%X' % (oct, mod, isel, addr, data))
	return outstr
	

def wrhash_parse(outstr, bitstr):
	oct = int(bitstr[6:3:-1], 2)		#bit[6:4]
	we = int(bitstr[14:6:-1], 2)		#bit[14:7], 1 bit for each mod
	comm_lsb = int(bitstr[23:14:-1], 2)	#bit[23:15]
	msb = int(bitstr[55:23:-1], 2)		#bit[55:24], 4 bits for each mod
	data = int(bitstr[151:55:-1], 2)	#bit[151:56], 12 bits for each mod
	outstr = outstr + ('oct=%d' % oct)
	i = 0
	while i < 8:
		if we & (1<<i):
			outstr = outstr + (' [mod%d: addr=%X data=%X]' % (i, (((msb>>(i<<2))&0xf)<<9)|comm_lsb, (data>>(i*12))&0xfff))
		i = i + 1
	return outstr


def wrbs_parse(outstr, bitstr):
	oct = int(bitstr[6:3:-1], 2)		#bit[6:4]
	we = int(bitstr[14:6:-1], 2)		#bit[14:7], 1 bit for each mod
	addr = int(bitstr[70:14:-1], 2)		#bit[70:15], 7 bits for each mod
	data = int(bitstr[150:70:-1], 2)	#bit[150:71], 10 bits for each mod
	outstr = outstr + ('oct=%d' % oct)
	i = 0
	while i < 8:
		if we & (1<<i):
			outstr = outstr + (' [mod%d: addr=%X data=%X]' % (i, (addr>>(i*7))&0x7f, (data>>(i*10))&0x3ff))
		i = i + 1
	return outstr


def rdtobuff_parse(outstr, bitstr):
	oct = int(bitstr[6:3:-1], 2)		#bit[6:4]
	re = int(bitstr[14:6:-1], 2)		#bit[14:7], 1 bit for each mod
	addr = int(bitstr[94:14:-1], 2)		#bit[94:15], 10 bits for each mod
	reset = int(bitstr[102:94:-1], 2)	#bit[102:95], 1 bit for each mod
	outstr = outstr + ('oct=%d' % oct)
	i = 0
	while i < 8:
		if (re & (1<<i)) or (reset & (1<<i)):
			outstr = outstr + (' [mod%d: addr=%X re=%d reset=%d]' % (i, (addr>>(i*10))&0x3ff, (re>>i)&0x1, (reset>>i)&0x1))
		i = i + 1
	return outstr


def rdlpm_parse(outstr, bitstr):
	quartet = int(bitstr[9:3:-1], 2)	#bit[9:4]
	isel = int(bitstr[14:10:-1], 2)		#bit[14:11]
	addr = int(bitstr[30:14:-1], 2)		#bit[30:15]
	outstr = outstr + ('quartet=%X isel=%d addr=%X' % (quartet, isel, addr))
	return outstr


def wrlpm_parse(outstr, bitstr):
	quartet = int(bitstr[9:3:-1], 2)	#bit[9:4]
	isel = int(bitstr[14:10:-1], 2)		#bit[14:11]
	addr = int(bitstr[30:14:-1], 2)		#bit[30:15]
	data = int(bitstr[143:30:-1], 2)	#bit[143:31]
	outstr = outstr + ('quartet=%X isel=%d addr=%X data=%X' % (quartet, isel, addr, data))
	return outstr
	

def ld2_parse(data):
	bitstr = ''
	for a in data:
		bitstr = bin(int(a, 16)|0x10)[-1:-5:-1] + bitstr

	assert len(bitstr) == 160, ('datalen is %d' % len(bitstr))

	cmd = ldcmd.get(getldcmd(bitstr), 'Unknown')
	outstr = 'RDBW, %s, ' % cmd
	if cmd == 'WrCombine':
		outstr = wrcombine_parse(outstr, bitstr)
	elif cmd == 'WrReg':
		outstr = wrreg_parse(outstr, bitstr)
	elif cmd == 'WrHash':
		outstr = wrhash_parse(outstr, bitstr)
	elif cmd == 'WrBS':
		outstr = wrbs_parse(outstr, bitstr)
	elif cmd == 'RdToBuffer':
		outstr = rdtobuff_parse(outstr, bitstr)
	elif cmd == 'WrLpm':
		outstr = wrlpm_parse(outstr, bitstr)
	else:
		outstr = outstr + 'nothing to parse'

	print(outstr, file=f_out)


def ld3_parse(addr, data):
	bitstr = ''
	for a in addr:
		bitstr = bin(int(a, 16)|0x10)[-1:-5:-1] + bitstr

	cmd = ldcmd.get(getldcmd(bitstr), 'Unknown')
	outstr = 'RDBR, %s, ' % cmd
	if cmd == 'RdReg':
		outstr = rdreg_parse(outstr, bitstr)
		outstr = outstr + (' rddata=%s' % data)
	elif cmd == 'RdLpm':
		outstr = rdlpm_parse(outstr, bitstr)
		outstr = outstr + (' rddata=%s' % data)
	else:
		outstr = outstr + 'nothing to parse'

	print(outstr, file=f_out)


if __name__ == '__main__':
	try:
		opts, args = getopt.getopt(sys.argv[1:], 'hi:o:')
	except getopt.GetoptError:
		print('\nError: getopt error!\n')
		usage()
		sys.exit(-1)

	if len(opts) == 0:
		print('\nError: no args specified!\n')
		usage()
		sys.exit(-1)

	for opt, arg in opts:
		if opt == '-h':
			usage()
			sys.exit(0)
		elif opt == '-i':
			input_file = arg
		elif opt == '-o':
			output_file = arg
		else:
			print('\nError: unrecognized option!\n') 
			sys.exit(-1)

	f_in = open(input_file, 'r')
	f_out = open(output_file, 'w')

	try:
		while True:
			rdbuf = f_in.readline()
			if not rdbuf:
				break

			L = rdbuf.split()
			if L[0] == 'LD':
				if L[1] == '5':		#LD 5 profile sbaddr width keyHexStr
					print('SBW, sbaddr=%s, width=%d, data=%s'
					% (L[3], int(L[4])+1, L[5]), file=f_out)
				elif L[1] == '2':	#LD 2 0 0 0 dataHexStr d=dev
					ld2_parse(L[5])
				elif L[1] == '3':	#LD 3 0 0 0 addr dataHexStr d=dev
					ld3_parse(L[5], L[6])
			elif L[0] == 'LC':		#LC profile sbaddr width keyHexStr ready match priority sec addata adwidth
				print('SBWC, profile=%s, sbaddr=%s, width=%d, key=%s,\
 ready=%s, match=%s priority=%s sec=%s addata=%s adwidth=%s'
				 % (L[1], L[2], int(L[3])+1, L[4], L[5], L[6], L[7], L[8], L[9], L[10]),
				 file=f_out)
			elif L[0] == 'NOP':
				print('NOP', file=f_out)
			else:
				print('don\'t care', file=f_out)
	finally:
		f_in.close()
		f_out.close()
		print('OVER')

sys.exit(0)
