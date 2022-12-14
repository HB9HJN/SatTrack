import socket
import serial
from time import sleep


ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1, parity=serial.PARITY_EVEN)

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

aaz = 0.0
eel = 0.0

mmaz = 0
mmel = 0

maz = '9999'
mel = '9999'

response = " "
while 1:
	data = conn.recv(BUFFER_SIZE)
	if not data:
		break

	#print("received data:", data)
	#os.system('clear')
	
	if(az > 360):
		aaz = 360
	else:
		aaz = az
	
	if(el > 90):
		eel = 90
	else:
		eel = el

	response = "{}\n{}\n".format(float(f'{az:.2f}'), float(f'{el:.2f}'))
	print(response)
	print("moving to az:{} el: {}".format( mmaz, mmel));
	print("mmoving to az:{} el: {}".format( maz, mel));
	print("responing with: \n {}".format(response))

	if(mmaz == 0):
		maz = '000' + str(mmaz)
	if(mmaz < 100):
		maz = '00' + str(mmaz)
	elif(mmaz < 1000):
		maz = '0' + str(mmaz)
	else:
		maz = str(mmaz)

	if(mmel == 0):
		mel = '000' + str(mmel)
	elif(mmel < 100):
		mel = '00' + str(mmel)
	elif(mmel < 1000):
		mel = '0' + str(mmel)
	else:
		mel  = str(mmel)
	
	if (data.startswith(b'P')):
		values = data.split(b' ')
		#print(values)
		mmaz = int(float(values[1])*10)-1800 +3600
		mmel = int(float(values[2])*10)

		#conn.send(b' ')
		conn.send(bytes(response, 'utf-8'))
	elif data == b'q\n' or data == b'S\n':
		print("close command, shutting down")
		conn.close()
		exit()
	elif data == b'p\n':
		conn.send(bytes(response, 'utf-8'))

	try:
		print(str(maz + mel))
		ser.reset_output_buffer()
		ser.write(bytes(maz + mel +'\r\n', 'ascii'))
		ser.reset_output_buffer()
	except:
		print('nosend')
		ser.reset_output_buffer()
		print(str(maz + mel))

	try:
		if(ser.in_waiting > 0 ):
			adc = ser.readline()
			ser.reset_input_buffer()
			ser.read_all()
			adc = adc.split(b'/')
			print(adc)
	except:
		print('nor')

	sleep(1)

	
	
