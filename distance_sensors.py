#!/usr/bin/env python3
"""
Copyright (c) 2025, BlackBerry Limited. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Ultrasonic Sensor Module for QNX using rpi_gpio.
 
This module demonstrates how to interface with the HC‑SR04 ultrasonic sensor on a QNX system.
It sends a trigger pulse, waits for the echo’s rising and falling edges, and calculates the distance
based on the duration of the echo pulse.
 
Wiring:
  - VCC: 5V
  - Trigger (US_TRIG): connected to GPIO 13
  - Echo (US_ECHO): connected to GPIO 25
  - GND: Ground
 
IMPORTANT:
  The sensor is powered by 5V, and the echo pin’s voltage is reduced by a resistor divider.
  This version disables the internal pull resistor (using GPIO.PUD_OFF) for the echo pin.
  
"""
 
import rpi_gpio as GPIO  # QNX Raspberry Pi GPIO module
import time
  
# Set GPIO pin 16 as an output pin
GPIO.setup(16, GPIO.OUT)
 
# Initially set GPIO pin 16 to LOW (turning it OFF)
GPIO.output(16, GPIO.LOW)

# Set GPIO pin 16 as an output pin
GPIO.setup(23, GPIO.OUT)
 
# Initially set GPIO pin 16 to LOW (turning it OFF)
GPIO.output(23, GPIO.LOW)

 
class UltrasonicSensor:
    """
    Class to manage the HC‑SR04 ultrasonic sensor.
    """
    # Speed of sound in cm/s
    SPEED_OF_SOUND = 34300  
 
    def __init__(self, trigger_pin=26, echo_pin=13, timeout=0.05):
        """
        Initializes the ultrasonic sensor by configuring the GPIO pins.
 
        :param trigger_pin: GPIO pin used for the trigger signal (default: 13).
        :param echo_pin: GPIO pin used for the echo signal (default: 25).
        :param timeout: Maximum time (in seconds) to wait for an echo (default: 0.05 s).
        """
        self.trigger_pin = trigger_pin
        self.echo_pin = echo_pin
        self.timeout = timeout
 
        # Use BCM numbering for GPIO pins.
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.trigger_pin, GPIO.OUT)
        # Disable the internal pull resistor on the echo pin (set to OFF)
        GPIO.setup(self.echo_pin, GPIO.IN, pull_up_down=GPIO.PUD_OFF)
 
        # Ensure the trigger is low to start.
        GPIO.output(self.trigger_pin, False)
        time.sleep(2)  # Allow the sensor time to settle.
 
    def send_trigger_pulse(self):
        """
        Sends a 10‑microsecond pulse on the trigger pin to initiate a measurement.
        Uses a busy‑wait loop for precise timing.
        """
        GPIO.output(self.trigger_pin, True)
        # Busy-wait for 10 microseconds using a high-resolution timer.
        start = time.perf_counter()
        while (time.perf_counter() - start) < 10e-6:
            pass
        GPIO.output(self.trigger_pin, False)
 
    def get_distance(self):
        """
        Measures the distance by sending a trigger pulse and timing the echo response.
 
        The function waits for the echo pin to go HIGH (rising edge) and then LOW (falling edge),
        calculates the duration of the echo pulse, and converts it to a distance in centimeters.
 
        :return: The measured distance in centimeters, or None if a timeout occurs.
        """
        self.send_trigger_pulse()
 
        # Wait for the echo pin to go HIGH (rising edge)
        start_time = time.perf_counter()
        while GPIO.input(self.echo_pin) == 0:
            if time.perf_counter() - start_time > self.timeout:
                print("Timeout waiting for rising edge")
                return None
        pulse_start = time.perf_counter()
 
        # Wait for the echo pin to go LOW (falling edge)
        while GPIO.input(self.echo_pin) == 1:
            if time.perf_counter() - pulse_start > self.timeout:
                print("Timeout waiting for falling edge")
                return None
        pulse_end = time.perf_counter()
 
        pulse_duration = pulse_end - pulse_start
 
        # Calculate distance: (pulse_duration * speed of sound) / 2.
        distance = (pulse_duration * self.SPEED_OF_SOUND) / 2.0
        return distance
 
    def cleanup(self):
        """
        Cleans up the GPIO settings.
        """
        GPIO.cleanup()
 
def main():
    """
    Main function to continuously measure and trigger buzzer pulses
    based on proximity to objects detected by two ultrasonic sensors.
    """
    sensor1 = UltrasonicSensor(trigger_pin=26, echo_pin=13)
    sensor2 = UltrasonicSensor(trigger_pin=6, echo_pin=5)

    try:
        while True:
            # Sensor 1 logic
            distance1 = sensor1.get_distance()
            if distance1 is not None:
                print("Distance of sensor 1: {:.2f} cm".format(distance1))
                if distance1 <= 50:
                    interval = max(0.05, (distance1 / 50.0) * 0.5)
                    GPIO.output(16, GPIO.HIGH)
                    time.sleep(interval)
                    GPIO.output(16, GPIO.LOW)
                    time.sleep(interval)
                else:
                    GPIO.output(16, GPIO.LOW)
            else:
                print("Error measuring distance 1.")

            # Sensor 2 logic
            distance2 = sensor2.get_distance()
            if distance2 is not None:
                print("Distance of sensor 2: {:.2f} cm".format(distance2))
                if distance2 <= 50:
                    interval = max(0.05, (distance2 / 50.0) * 0.5)
                    GPIO.output(23, GPIO.HIGH)
                    time.sleep(interval)
                    GPIO.output(23, GPIO.LOW)
                    time.sleep(interval)
                else:
                    GPIO.output(23, GPIO.LOW)
            else:
                print("Error measuring distance 2.")

            # Small delay between cycles
            time.sleep(0.05)

    except KeyboardInterrupt:
        print("Measurement stopped by user.")
    finally:
        sensor1.cleanup()
        sensor2.cleanup()


if __name__ == '__main__':
    main()
