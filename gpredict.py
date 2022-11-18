import socket
import serial
import os

ser = serial.Serial('/dev/ttyACM0', 9600, timeout=0, parity=serial.PARITY_EVEN)

TCP_IP = '127.0.0.1'
TCP_PORT = 4533
BUFFER_SIZE = 100

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((TCP_IP, TCP_PORT))
s.listen(1)

conn, addr = s.accept()
print('Connection address:', addr)

az = 0.0
el = 0.0

maz = '0000'
mel = '0000'

response = " "
while 1:
	data = conn.recv(BUFFER_SIZE)
	if not data:
		break

	#print("received data:", data)
	os.system('clear')

	print("moving to az:{} el: {}".format( maz, mel));
	print("responing with: \n {}".format(response))
	
	if (data.startswith(b'P')):
		values = data.split(b' ')
		#print(values)
		maz = int(float(values[1])*10)
		mel = int(float(values[2])*10)

		if(maz == 0):
			maz = '000' + str(maz)
		if(maz < 100):
			maz = '00' + str(maz)
		elif(maz < 1000):
			maz = '0' + str(maz)

		if(mel == 0):
			mel = '000' + str(mel)
		elif(mel < 100):
			mel = '00' + str(mel)
		elif(mel < 1000):
			mel = '0' + str(mel)

		conn.send(b' ')
	elif data == b'q\n' or data == b'S\n':
		print("close command, shutting down")
		conn.close()
		exit()
	elif data == b'p\n':

		response = "{}\n{}\n".format(az, el)
		conn.send(bytes(response, 'utf-8'))

	try:
		ser.write(bytes(maz + mel +'\r\n', 'ascii'))
	except:
		pass

	adc = ser.readline()
	adc = adc.split(b'/')
	ser.reset_input_buffer()
	ser.reset_output_buffer()
	try:
		az = float(adc[0])
		el = float(adc[1])
		tp = adc[2]
		#print("REAL azimuth:{}, elevation;{}, tp:{}".format(az, el, tp))
	except:
		pass

	
	
