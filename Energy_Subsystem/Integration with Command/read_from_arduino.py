import serial, time 

arduino = serial.Serial('/dev/cu.usbmodem1461301', 9600, timeout=.1) #TODO: change this to proper computer

time.sleep(1) #give the connection a second to settle
arduino.write(("Hello from Python!").encode('utf-8'))

while True:
	data = arduino.readline()[:-2] #the last bit gets rid of the new-line chars
	if data:
		print(data.decode('utf-8')) #strip out the new lines for now
		# (better to do .read() in the long run for this reason