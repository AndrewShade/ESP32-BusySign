# ESP32-BusySign

This project provides a low power solution for a remote status indicator. It is designed to signal whether a home office is busy or available without requiring a traditional network. The system consists of a battery powered receiver with an LED strip and a desktop mounted sender with a physical toggle button.

## Architecture

The project uses the ESP-NOW protocol for direct communication between two ESP32 microcontrollers. To maximize battery life on the receiver, the system employs a specific timing strategy instead of maintaining a constant connection.

The receiver spends most of its time in deep sleep. Every 10 seconds, it wakes up for 100 milliseconds to listen for incoming packets. To ensure the message is caught during this small window, the sender repeats the state change packet for 11 seconds once the button is pressed. This overlapping timing ensures that even if the receiver just entered sleep, it will catch the message during its next wake cycle.

## Hardware Requirements

* Receiver: ESP32 development board, Adafruit NeoPixel strip, and a 3.7V LiPo battery.
* Sender: ESP32-S3 or similar, a momentary push button, and a built-in or external NeoPixel for feedback.

## Project Structure

* receiver.ino: Handles the deep sleep cycles and the 100ms listening window. It uses RTC memory to remember the LED state even when the main processor is powered down.
* sender.ino: Manages the button debouncing and the 11 second packet broadcast. It also provides visual feedback on the sender status using a local LED.

## Setup and Installation

1. Get the Receiver MAC Address
Find the MAC address of your receiver board by running a scanner sketch on the receiver ESP32.

2. Configure the Sender
Open sender.ino and locate the receiverMAC array. Replace the hex values with the MAC address you found in the previous step.
```C+
uint8_t receiverMAC[] = { 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX };
```

3. Wiring
Connect the receiver NeoPixel data pin to GPIO 2. Connect the sender button to GPIO 4 and ground. The sender code is configured to use the built-in NeoPixel on pin 48 for the ESP32-S3.

4. Upload
Flash the respective code to each board. Ensure you have the Adafruit NeoPixel library installed in your environment.

## Features

* Zero Latency Feedback: The sender updates its local LED immediately so you know the toggle was successful.
* State Persistence: The receiver stores the current status in RTC memory. If it wakes up and sees no new instructions, it restores the previous light color.
* Energy Efficiency: By using deep sleep and disabling the radio between checks, the receiver can run for several weeks on a single charge.
* Reliable Communication: The 11 second broadcast window prevents missed signals caused by sleep synchronization.
