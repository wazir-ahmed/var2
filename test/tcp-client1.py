import socket
import sys
import time

a = list([0 for i in range(0, 5)])
b = list([0 for i in range(0, 10)])
c = list([0 for i in range(0, 10)])
d = list([0 for i in range(0, 10)]) 

#------------------------------

#ver - 1 | #cmd - 5
a[0] = int('00010101', 2)

#addr - 15104085 | cont_f - 0
a[1] = int('00000001', 2)
a[2] = int('11001100', 2)
a[3] = int('11110000', 2)
a[4] = int('10101010', 2)

#------------------------------

#ver - 1 | #cmd - 4
b[0] = int('00010100', 2)

#addr - 15104085 | cont_f - 1
b[1] = int('00000001', 2)
b[2] = int('11001100', 2)
b[3] = int('11110000', 2)
b[4] = int('10101011', 2)

#payload_len - 4 
b[5] = int('00000000', 2)
b[6] = int('00000000', 2)
b[7] = int('00000000', 2)
b[8] = int('00000100', 2)

#content_t - 2
b[9] = int('00000010', 2)

#E_size - 1
c[0] = int('00000000', 2)
c[1] = int('00000001', 2)

#E_value - 15
c[2] = int('00001111', 2)

#payload_value - 209919
c[3] = int('00000011', 2)
c[4] = int('00110011', 2)
c[5] = int('11111111', 2)

#------------------------------


#ver - 1 | #cmd - 4
c[6] = int('00010100', 2)

#addr - 15104085 | cont_f - 1
c[7] = int('00000001', 2)
c[8] = int('11001100', 2)
c[9] = int('11110000', 2)
d[0] = int('10101011', 2)

#payload_len - 2
d[1] = int('00000000', 2)
d[2] = int('00000000', 2)
d[3] = int('00000000', 2)
d[4] = int('00000010', 2)

#content_t - 2
d[5] = int('00000010', 2)

#E_size - 0
d[6] = int('00000000', 2)
d[7] = int('00000000', 2)

#payload_value - 13311 
d[8] = int('00110011', 2)
d[9] = int('11111111', 2)

#------------------------------

data1_int = int.from_bytes(a, byteorder='big')
data1_bytes = data1_int.to_bytes(len(a), byteorder='big', signed=False)

data2_int = int.from_bytes(b, byteorder='big')
data2_bytes = data2_int.to_bytes(len(b), byteorder='big', signed=False)

data3_int = int.from_bytes(c, byteorder='big')
data3_bytes = data3_int.to_bytes(len(c), byteorder='big', signed=False)

data4_int = int.from_bytes(d, byteorder='big')
data4_bytes = data4_int.to_bytes(len(d), byteorder='big', signed=False)

#------------------------------
sock_address = ('127.0.0.1', 1122)

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

sock.connect(sock_address)

sock.send(data2_bytes)
time.sleep(2)
sock.send(data3_bytes)
time.sleep(2)
sock.send(data4_bytes)
time.sleep(2)
sock.send(data1_bytes)

print("------------------")
for i in range(0, 14):
	data = sock.recv(1)
	if data:
		print (int.from_bytes(data, byteorder='big'))
	

sock.close()