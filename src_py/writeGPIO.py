import RPi.GPIO as GPIO
from time import sleep

GPIO.setmode(GPIO.BCM)

GPIO.setup(9, GPIO.OUT)
GPIO.setup(11, GPIO.OUT)

while 1:
  GPIO.output(9, True)
  sleep(0.25)
  GPIO.output(9, False)
  sleep(0.25)

  GPIO.output(11, True)
  sleep(0.25)
  GPIO.output(11, False)
  sleep(0.25)

  #GPIO.output(11, True)
  #sleep(0.25)
  #GPIO.output(11, False)
  #sleep(0.25)

  #GPIO.output(11, True)
  #sleep(0.25)
  #GPIO.output(11, False)
  #sleep(0.25)

GPIO.cleanup()
