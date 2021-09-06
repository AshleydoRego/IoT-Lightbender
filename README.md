# IoT-Lightbender
An arduino based IoT Switchboard for lights, fan control via an MQTT Server

**Motivation:**

I have a typical morning routine. Every day, I wake up turn my geyser on, brush my teeth, perform my morning ablutions, make myself a cup of coffee, some breakfast, and head to work. And every morning, without fail, half an hour after leaving the house, I'll get an enraged call from either one of my flatmates threatening to make me pay the entire electricity bill for the month. Because as usual, I've left my AC, my geyser, my fan, my lights, all of them, on. 🤦‍♂️
I will admit, I do have a very bad habit. I always forget to turn my lights off when I leave the room. I know it's something I should change. If I was pitching this idea to someone, here's where I'd tell them that it’s great for the environment and all that, but really, I just need my flatmates to get off my case 😋
And while I know this is a problem for me, I'm also too lazy to bother myself with changing my ways. I'm an engineer, I'd rather fix my problems with over-the-top, barely necessary tech. So, I decided to make an IoT Switchboard which would allow me to see whether my lights were turned on or not and control them from anywhere in the world, via an app on my phone.

**Requirements:**

For my remote light control project, I identified the following essential requirements:
1.	Control Light: These is a simple on/off lights that can be controlled using a relay between a GPIO pin on the microcontroller and the high voltage ac circuit. 
2.	Control Fan and dimmable light: In addition to the on/off function, the fan and dimmable bulb control also needs AC voltage modulation to adjust the speed/brightness. This can be accomplished using a zero-crossing detector circuit and a triac for phase/angle control. 
3.	Control Mood Lights: RGB LED strip controlled by three buttons. One for on/off, one to cycle through colour and one to cycle through brightness.
4.	Remote Control: The entire system must be able to be controlled remotely through the internet/phone app
5.	Must have physical button: To allow operation in person without need for a phone


**Required Components**


1.	1x Arduino Uno Wifi Rev2
2.	1x Arduino Uno
3.	1x 5V 4 Channel Relay Board
4.	2x AC Light Lamp Dimming LED Lamp and Motor Dimmer Module, 1 Channel-5A
5.	1x DC-DC Step-Down Buck Converter Power Supply Module 24V 12V 9V to 5V 5A 25W
6.	1x 12V 6A AC-DC Switching Power Supply Module
7.	1x 0.91 inch 128×32 Blue OLED Display Module with I2C/IIC Serial Interface
8.	8x Tactile Push Button Switch 12x12x12mm
9.	2x M274 360-degree Rotary Encoder
10.	Connecting wires
11.	2x Perfboard 
12.	Header pins


**Schematics**

Switchboard

![Image of Yaktocat](https://octodex.github.com/images/yaktocat.png)




