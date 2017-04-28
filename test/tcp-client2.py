import socket
import sys
import time

a = list([0 for i in range(0, 5)]) 
b = list([0 for i in range(0, 5)]) 
c = list([0 for i in range(0, 5)]) 

#------------------------------

#ver - 1 | #cmd - 6
a[0] = int('00010110', 2)

#addr - 15104085 | cont_f - 0
a[1] = int('00000001', 2)
a[2] = int('11001100', 2)
a[3] = int('11110000', 2)
a[4] = int('10101010', 2)

#------------------------------

#ver - 1 | #cmd - 6
b[0] = int('00010110', 2)

#addr -  | cont_f - 0
b[1] = int('00000000', 2)
b[2] = int('11001100', 2)
b[3] = int('11110000', 2)
b[4] = int('10101010', 2)

#------------------------------

#ver - 1 | #cmd - 7
c[0] = int('00010111', 2)

#addr -  | cont_f - 0
c[1] = int('00000001', 2)
c[2] = int('11001100', 2)
c[3] = int('11110000', 2)
c[4] = int('10101010', 2)

#------------------------------

data1_int = int.from_bytes(a, byteorder='big')
data1_bytes = data1_int.to_bytes(len(a), byteorder='big', signed=False)

data2_int = int.from_bytes(b, byteorder='big')
data2_bytes = data2_int.to_bytes(len(a), byteorder='big', signed=False)

data3_int = int.from_bytes(c, byteorder='big')
data3_bytes = data3_int.to_bytes(len(a), byteorder='big', signed=False)

#------------------------------

sock_address = ('127.0.0.1', 1122)

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

sock.connect(sock_address)

time.sleep(2)
sock.send(data1_bytes)

print("------------------")
while True:
	data = sock.recv(1)
	if data:
		print (int.from_bytes(data, byteorder='big'))

sock.close()