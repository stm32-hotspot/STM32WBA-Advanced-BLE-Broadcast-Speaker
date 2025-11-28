#!/usr/bin/env python3
##############################################################################
# @file    sf2TowSff.py
# @author  convert an sf2 file in PCM in SFF file ( sample in float)
# @brief   Dev tool
##############################################################################
# @attention
#
# <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
# All rights reserved.</center></h2>
#
# This software component is licensed by ST under Ultimate Liberty license
# SLA0044, the "License"; You may not use this file except in compliance with
# the License. You may obtain a copy of the License at:
#                             www.st.com/SLA0044
#
##############################################################################
# sf2tosff florestan-subset.sf2 florestan-subset.sff
# bin2hex --offset 0x90300000 buildStruct.bin X-CUBE-Audio-kit-STM32H573I-DK-florestan-full.sf2-0x90300000.hex
# package test.pkt --bin florestan-full.sf2
import sys
import os
import argparse
from ctypes import *
import shutil
import numpy as np
import struct
from intelhex import bin2hex
import ctypes
from colorama import Fore, Back, Style


def printC(*args, sep=' '):
    print(Fore.YELLOW, sep.join("{}".format(a) for a in args), Style.RESET_ALL)


class Bin2C:
	def __init__(self, args):
		self.args = args
		self.args._aligned = 4
		self.args._section = ""
		self.args._nbCharByLine=32
		self.process()

	def normalizeC(self,fileName):
		normalized = ""
		for elem in fileName:
			if elem >= '0' and elem <= '9':
				normalized += elem
			elif elem >= 'A' and elem <= 'Z':
				normalized += elem
			elif elem >= 'a' and elem <= 'z':
				normalized += elem
			elif elem == '.':
				normalized += '_'
			else:
				normalized += '_'
		if normalized[0] >= '0' and normalized[0] <= '9':
			normalized = "_" + normalized
		return normalized


	def prologue(self,hFile, name, size):
		hFile.write("#ifndef  _" + name + "_\n")
		hFile.write("#define  _" + name + "_\n")
		if self.args._aligned != 0:
			hFile.write("#if !defined ( __ICCARM__ )\n")
			hFile.write("__attribute__((aligned(" + str(self.args._aligned) + ")))\n")
			hFile.write("#endif\n")
		if self.args._section != "":
			hFile.write("#if defined ( __ICCARM__ )\n")
			hFile.write("#pragma location=\"" + self.args._section + "\"\n")
			hFile.write("#else\n")
			hFile.write("__attribute__((section(\"." + self.args._section + "\")))\n")
			hFile.write("#endif\n")
			string = "const unsigned char " + name + "[" + str(size) + "]={\n"
		else:
			string = "const unsigned char " + name + "[" + str(size) + "]={\n"
		hFile.write(string)

	def epilogue(self,hFile, name, size):
		string = "\n};"
		hFile.write(string)
		hFile.write("\n#endif  /* _" + name + "_ */\n")

	def process(self):
		binFile = self.args.inFile;
		baseName = os.path.basename(binFile)
		cLabel = self.normalizeC(self.args.array_name)
		dumpFile = self.args.out
		print("Convert : ",binFile,' as ',dumpFile)

		try:
			with open(binFile, 'rb') as i:
				data = i.read()
		except IOError:
			print("File not found:", str(binFile))
			exit(-1)
		try:
			with open(dumpFile, 'w') as o:
				lineCount = 0
				curChar = 0
				self.prologue(o, cLabel, len(data))
				o.write("\t")
				for elem in data:
					if lineCount + 1 > self.args._nbCharByLine:
						o.write("\r\t")
						lineCount = 0
					if curChar != 0:
						o.write(",")
					else:
						o.write(" ")
					# key = str(elem).encode("hex").upper()
					key = "0x{0:02x}".format(elem)
					o.write(key)
					lineCount += 1
					curChar += 1
				self.epilogue(o, cLabel, len(data))
				o.close()

		except IOError:
			print("Can't Write :", str(dumpFile))
			exit(-1)
		i.close()



class Bin2Hex:
	def __init__(self, args):
		self.args = args
		self.process()

	def process(self):
		print("convert in hex : ",self.args.inFile," at ",self.args.offset)
		base = 10
		offset = 0
		if self.args.offset[:2].lower() == '0x':
			base = 16
		try:
			offset = int(self.args.offset, base)
		except:
			print("Wrong offset")
		bin2hex(self.args.inFile, self.args.outFile,offset)


class phdr(ctypes.Structure):
    _pack_   = 1
    _fields_ = [("achPresetName", c_int8*20),
                ("wPreset", c_uint16),
                ("wBank", c_uint16),
                ("wPresetBagNdx",c_uint16),
                ("dwLibrary", c_uint32),
                ("dwGenre", c_uint32),
                ("dwMorphology", c_uint32)
                ]



class Sf2ToSff:

	def __init__(self, args):
		self.args = args
		self.tCheckList=[]
		self.process()

	def copyBody(self,name,size):
		self.szRam += size
		body = self.fileIn.read(size)
		if name == "INAM":
			print("version    :",body.decode('utf-8'))
		if name == "ICRD":
			print("date       :",body.decode('utf-8'))
		if name == "IENG":
			print("Author     :",body.decode('utf-8'))
		if name == "IPRD":
			print("Product    :",body.decode('utf-8'))
		if name == "ICOP":
			print("Copyright  :",body.decode('utf-8'))
		if name == "ICMT":
			print("Comment    :",body.decode('utf-8'))
		if name == "ISMT":
			print("Tool       :",body.decode('utf-8'))
		if name == "phdr":
			szChunk = size
			szPreset = ctypes.sizeof(phdr)
			index = 0;
			nbPreset = size/szPreset
			print("Preset Number : ",nbPreset)

			while (szChunk > 0):
				preset = body[index*szPreset:(index+1)*szPreset]
				info = phdr.from_buffer_copy(preset)
				s = bytearray(info.achPresetName)
				print("\t",index,"Preset : ",info.wPreset+1," ",bytearray(info.achPresetName).decode('utf-8'))
				szChunk-= szPreset
				index += 1

		self.fileOut.write(body)
		return size

	def readSmpl(self,size):
		body = self.fileIn.read(size)
		data16 = np.frombuffer(body,dtype=np.int16)
		if self.args.verbose :
			print(" Convert Samples from 16 bits to float : ",end='')
		dataFloat = bytearray()
		cpt = 0
		for i in data16:
			if cpt > 100000:
				cpt = 0
				if self.args.verbose:
					print(".",end='')
			dataFloat.extend(bytearray(struct.pack("f", i/32767.0)))
			cpt = cpt +1

		if self.args.verbose :
			print("")
		curPos = self.fileOut.tell()
		alignArray = bytearray(0);
		if (curPos & 0x3) != 0:
			alignArray = bytearray(4-(curPos & 0x3))
		newsize = len(dataFloat) + len(alignArray)

		curPos = self.fileOut.tell()
		self.fileOut.seek(-8,1)
		newChunk=b'smpf'
		self.fileOut.write(newChunk)
		self.fileOut.write(newsize.to_bytes(4, 'little'))
		self.fileOut.write(alignArray)
		self.fileOut.write(dataFloat)
		self.szWavFloat = len(dataFloat)
		return newsize

	def isRoot(self,name):
		if name == "sfbk":
			return True
		if name == "INFO":
			return True
		if name == "sdta":
			return True
		if name == "pdta":
			return True

		return False

	def readIFF(self,nameParent,sizeChunk,tab):
		sizenew = 0
		sizeParent = 0
		while sizeParent < sizeChunk :
			pos, name, size = self.openChunk()
			if self.isRoot(name):
				if self.args.verbose:
					print(tab,"Root:", name)
				self.fileIn.seek(-4, 1)
				self.fileOut.seek(-4, 1)
				sizenew  += 4
				sizeParent+=4
				continue
			if self.args.verbose:
				print(tab,"Chunk:",name,"Size:",size)

			if name=="LIST":
				posListStart = self.fileOut.tell()
				sizenew += self.readIFF(name,size,tab+"\t")
				self.closeChunk(posListStart)
			elif name=="smpl": # 16 bits version raw
				sizenew += self.readSmpl(size)
			elif name=="smpf": # float  version raw
				sizenew += self.copyBody(name,size)
			else:
				sizenew += self.copyBody(name,size)
			sizeParent+= size + 8 #size + chunk
			sizenew += 8 # size chunk + size
		return sizenew

	def openChunk(self):
		name = self.fileIn.read(4)
		size = self.fileIn.read(4)
		self.fileOut.write(name)
		self.fileOut.write(size)
		pos =  self.fileOut.tell()
		return pos,name.decode('utf-8'),int.from_bytes(size, byteorder='little')


	def closeChunk(self,posStart):
		curPos = self.fileOut.tell()
		size = curPos - posStart
		self.fileOut.seek(posStart - 4,0)
		self.fileOut.write(size.to_bytes(4, 'little'))
		self.fileOut.seek(curPos,0)

	def process(self):
		print("Convert standard sf2 to sf(float) file : ",self.args.inFile)
		try:
			self.fileIn = open(self.args.inFile, "rb")
			self.fileOut = open(self.args.outFile, "wb+")
			self.szRam = 0
			self.szWavFloat = 0
			pos,name,sizeParent = self.openChunk()
			self.readIFF(name,sizeParent,"")
			self.closeChunk(pos)
			self.fileIn.close()
			self.fileOut.close()
			printC("\tRam   Mandatory Approximation: ",int(self.szRam/1024),"KB")
			printC("\tFlash Mandatory Approximation: ",int(self.szWavFloat/1024),"KB")


		except ValueError:
			print("read error: ",self.args.inFile," ",ValueError)



class Package:
	def __init__(self, args):
		self.args = args
		self.process()

	def process(self):
		print("Package files  using a signature: ",self.args.signature)
		self.signature = 0
		if self.args.signature[:2].lower() == '0x':
			base = 16
		try:
			self.signature = int(self.args.signature, base)
		except:
			print("Wrong signature")

		size = 0
		try:
			print("Writing : ", self.args.outFile)
			self.fileOut = open(self.args.outFile, "wb+")
			self.fileOut.write(self.signature.to_bytes(4,'little'))
			self.fileOut.write(size.to_bytes(4,'big'))
			tOffset = [len(self.args.bins)]
			for i in range(len(self.args.bins)):
				self.fileOut.write(tOffset[i].to_bytes(4,'little'))

			for i in range(len(self.args.bins)):
				tOffset[i] =self.fileOut.tell()
				print("Reading : ",self.args.bins[i])
				self.fileIn = open(self.args.bins[i], "rb")

				bin = self.fileIn.read()
				self.fileOut.write(bin)
				self.fileIn.close()
			size = self.fileOut.tell()
			self.fileOut.seek(4,0)
			self.fileOut.write(size.to_bytes(4,'little'))
			for i in range(len(self.args.bins)):
				self.fileOut.write(tOffset[i].to_bytes(4,'little'))
			self.fileOut.close()
		except ValueError:
			print("Error: ",self.args.outFile," ",ValueError)



subcmds = {
	'sf2tosff': Sf2ToSff,
	'bin2c': Bin2C,
	'bin2hex': Bin2Hex,
	'package' : Package
}


def main():
	parser = argparse.ArgumentParser()
	subs = parser.add_subparsers(help='subcommand help', dest='subcmd')
	sf2toSff  = subs.add_parser('sf2tosff', help='Convert an sf2 file in PCM in SFF file ( sample in float)')
	sf2toSff.add_argument("inFile")
	sf2toSff.add_argument("outFile")
	sf2toSff.add_argument('--verbose',default=False,help='print verbose info')

	bin2c  = subs.add_parser('bin2c', help='Convert an binary to a C file ')
	bin2c.add_argument("inFile")
	bin2c.add_argument('--out',default="structFile.c",help='C File generated')
	bin2c.add_argument('--array_name',default="structFile",help='Variable name for the array')


	bin2hex  = subs.add_parser('bin2hex', help='Convert an binary to a intelHex file')
	bin2hex.add_argument("inFile")
	bin2hex.add_argument("outFile")
	bin2hex.add_argument('--offset',default="0x80000000",help='Offset in hex,default 0x80000000 ')


	package  = subs.add_parser('package', help='Package a list of binaries')
	package.add_argument("outFile")
	package.add_argument('--signature',default="0x42324653",help='Offset in hex,default 0x42324653 ')
	package.add_argument('--bins',action='append', default=[],help='Add a binary file to the list')




	args = parser.parse_args()
	if args.subcmd is None:
		print('Must specify a subcommand')
		sys.exit(1)
	subcmds[args.subcmd](args)


if __name__ == "__main__":
	main()
