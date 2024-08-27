from machine import Pin, time_pulse_us
import time

# Define the trigger and echo pins
trigger_pin = Pin(5, Pin.OUT)
echo_pin = Pin(4, Pin.IN)

# Define a function to measure the distance
def measure_distance():
    # Trigger the sensor by sending a 10 microsecond pulse
    trigger_pin.on()
    time.sleep_us(10)
    trigger_pin.off()
    
    # Measure the duration of the echo pulse
    duration = time_pulse_us(echo_pin, 1, 30000)
    
    # Convert the duration to distance in centimeters
    distance = duration / 58
    
    return distance

while True:
    distance = measure_distance()
    print(int(distance))
    time.sleep(.3)