import serial

ser = serial.Serial('/dev/ttyACM0', 9600)

while 1:
  line = '123'
  ser.writeline(line)
  print(line)
