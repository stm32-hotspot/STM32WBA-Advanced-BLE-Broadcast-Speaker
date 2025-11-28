
##############################################################################
# @file    localServer-127.0.0_1234.py
# @author  MCD Application Team
# @version V1.0.0
# @date    2/28/2019
# @brief   Implements a basic server that simulate an aws serverless lambda function locally
#		   the server uses the aws lambda implementation (lambda_function.py), run the computation and returns the result exactly like aws
# 		   but the compilation is done locally
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


import signal
import sys
from http.server import HTTPServer, BaseHTTPRequestHandler
from lambda_function import *



url  = '127.0.0.1'	# url localhost
port = 1234			# connection port
cptCnx = 0;			# connection counter
bVerbose = False		# prints the result


def __close_all(comment=""):
    print('todo. CLOSE ALL !')


def __signal_handler(sig, frame):
    print('You pressed Ctrl+C !')
    __close_all(comment=" (interrupted by user)")
    sys.exit(0)


# register call-back for Ctrl+C capture
signal.signal(signal.SIGINT, __signal_handler)


# implements a basic WEB HTTP service

class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
	# Add the CORS service to pass through security (Cross-origin resource sharing)
	def end_headers(self):
		self.send_header('Access-Control-Allow-Origin', '*')
		self.send_header('Access-Control-Allow-Methods', 'GET')
		self.send_header('Cache-Control', 'no-store,no-cache,must-revalidate')
		return super(SimpleHTTPRequestHandler,self).end_headers();

	# lambda uses a POST
	def do_POST(self):
		statusCode  = 404
		content_length = int(self.headers['Content-Length'])
		if content_length:
			# read the body
			body = self.rfile.read(content_length).decode('utf-8')
			print(cptCnx,"Received: ",body)
			# build the request and send it to the lambda function
			request = {"body": body}
			result = lambda_handler(request,0)
			#build and send the response
			statusCode = result['statusCode']
			self.send_response(statusCode)
			self.end_headers()
			if statusCode  == 200:
				#if oK, build the response body
				strJson = result["body"].encode('utf-8');
				#send the response body
				self.wfile.write(strJson)
				print(cptCnx, "Computation: Success")
				if bVerbose == True:
					print("Result:\n")
					print(result["body"],"\n")
				else:
					print("Result size:", len(strJson));
			else:
				print(cptCnx, "Computation: Failure")
			print(flush=True)

print("Lambda server listening at ",url,":",port)
httpd = HTTPServer((url, port), SimpleHTTPRequestHandler)
httpd.serve_forever()