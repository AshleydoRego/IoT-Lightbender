# IoT-Lightbender
An arduino based IoT Switchboard for lights, fan control via an MQTT Server

## **Motivation:**

I have a typical morning routine. Every day, I wake up turn my geyser on, brush my teeth, perform my morning ablutions, make myself a cup of coffee, some breakfast, and head to work. And every morning, without fail, half an hour after leaving the house, I'll get an enraged call from either one of my flatmates threatening to make me pay the entire electricity bill for the month. Because as usual, I've left my AC, my geyser, my fan, my lights, all of them, on. 🤦‍♂️
I will admit, I do have a very bad habit. I always forget to turn my lights off when I leave the room. I know it's something I should change. If I was pitching this idea to someone, here's where I'd tell them that it’s great for the environment and all that, but really, I just need my flatmates to get off my case 😋
And while I know this is a problem for me, I'm also too lazy to bother myself with changing my ways. I'm an engineer, I'd rather fix my problems with over-the-top, barely necessary tech. So, I decided to make an IoT Switchboard which would allow me to see whether my lights were turned on or not and control them from anywhere in the world, via an app on my phone.

## **Requirements:**

For my remote light control project, I identified the following essential requirements:
1.	Control Light: These is a simple on/off lights that can be controlled using a relay between a GPIO pin on the microcontroller and the high voltage ac circuit. 
2.	Control Fan and dimmable light: In addition to the on/off function, the fan and dimmable bulb control also needs AC voltage modulation to adjust the speed/brightness. This can be accomplished using a zero-crossing detector circuit and a triac for phase/angle control. 
3.	Control Mood Lights: RGB LED strip controlled by three buttons. One for on/off, one to cycle through colour and one to cycle through brightness.
4.	Remote Control: The entire system must be able to be controlled remotely through the internet/phone app
5.	Must have physical button: To allow operation in person without need for a phone


## **Required Components**


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


## **Schematics**

### Switchboard

![](https://github.com/AshleydoRego/IoT-Lightbender/blob/main/Images/Switchboard%20Schematics.jpg)

### Circuit Board

![](https://github.com/AshleydoRego/IoT-Lightbender/blob/main/Images/Circuit%20Board%20schematic.jpg)

### AC Mains

![](https://github.com/AshleydoRego/IoT-Lightbender/blob/main/Images/AC%20Mains%20Schematic.jpg)

## **Instructions: Device**

### Switchboard

Solder the buttons, encoders and display as shown in the switchboard schematic

![](https://github.com/AshleydoRego/IoT-Lightbender/blob/main/Images/SwitchboardUnit.jpg)

### Circuit Board

Solder the header pins to make the main circuit board according to the schematics

![](https://github.com/AshleydoRego/IoT-Lightbender/blob/main/Images/CircuitBoard.jpg)

### Main Circuit

Mount the components on a board and make the AC connections according to the schematic

![](https://github.com/AshleydoRego/IoT-Lightbender/blob/main/Images/MountedComponents.jpg)

Connect the Arduino pins to the appropriate header pins on the circuit board, relay board and dimmer circuits according to the schematic

![](https://github.com/AshleydoRego/IoT-Lightbender/blob/main/Images/Arduino%20Connections.jpg)

Upload the code provided. Arduino Uno Wifi Rev2 is the master and Arduino UNO is the slave

Connect the live wires of the fan, dimmable bulb, regular bulb and plug point to the NO port on the appropriate relay. Connect all neutral lines to the common neutral

![](https://github.com/AshleydoRego/IoT-Lightbender/blob/main/Images/FinalDevice.jpg)

Place device behind current switchboard and replace the current switchboard with the one just built.

![](https://github.com/AshleydoRego/IoT-Lightbender/blob/main/Images/Finished%20Swithboard.jpg)

You should now be able to control the lights and fans via the physical buttons and knobs.

## **Instructions MQTT**

Set up a MQTT server using the service of your choice. I used HiveMQ. Set up the access credentials and place them in the appropriate location in the code for the Arduino uno wifi rev 2
Next use any MQTT client app, I used ‘MQTT Dash’, available on the play store. Setup widgets on the app with the following topics and messages

### Dimmable Bulb Control
#### On/Off
PUBLISH TOPIC = ‘ToDevice/Switchboard/BULB1’

SUBSCRIBE TOPIC = ‘ToDevice/App/BULB1’

MESSAGE = ‘ON’ or ‘OFF’

ACTION: Will turn the bulb on or off.

#### Brightness

PUBLISH TOPIC = ‘ToDevice/Switchboard/BULB1/DIM’

SUBSCRIBE TOPIC = ‘ToDevice/App/BULB1/DIM’

MESSAGE = ‘X’ – where X is any integer between 0-100

ACTION: Will set the bulb brightness percentage to the integer passed in the message.

### Fan Control

#### On/Off

PUBLISH TOPIC = ‘ToDevice/Switchboard/FAN1’

SUBSCRIBE TOPIC = ‘ToDevice/App/FAN1'

MESSAGE = ‘ON’ or ‘OFF’

ACTION: Will turn the fan on or off.

#### Brightness

PUBLISH TOPIC = ‘ToDevice/Switchboard/FAN1/DIM’

SUBSCRIBE TOPIC = ‘ToDevice/App/FAN1/DIM’

MESSAGE = ‘X’ – where X is any integer between 0-100

ACTION: Will set the fan speed percentage to the integer passed in the message.

### Regular Bulb Control

#### On/Off

PUBLISH TOPIC = ‘ToDevice/Switchboard/BULB2’

SUBSCRIBE TOPIC = ‘ToDevice/App/BULB2’

MESSAGE = ‘ON’ or ‘OFF’

ACTION: Will turn the bulb on or off.

### Plug Point Control

#### On/Off

PUBLISH TOPIC = ‘ToDevice/Switchboard/PLUG'

SUBSCRIBE TOPIC = ‘ToDevice/App/PLUG’

MESSAGE = ‘ON’ or ‘OFF’

ACTION: Will turn the plugpoint on or off.

### LED Control
#### On/Off
PUBLISH TOPIC = ‘ToDevice/Switchboard/LED'

SUBSCRIBE TOPIC = ‘ToDevice/App/LED’

MESSAGE = ‘ON’ or ‘OFF’

ACTION: Will turn the LED strip on or off.

#### Brightness
PUBLISH TOPIC = ‘ToDevice/Switchboard/LED/BRIGHTNESS’

SUBSCRIBE TOPIC = ‘ToDevice/App/LED/BRIGHTNESS’

MESSAGE = ‘X’ – where X is any integer between 0-100

ACTION: Will set the LED brightness percentage to the integer passed in the message.

#### Colour
PUBLISH TOPIC = ‘ToDevice/Switchboard/LED/COLOUR’

SUBSCRIBE TOPIC = ‘ToDevice/App/LED/COLOUR’

MESSAGE = ‘X’ – where X is a hex colour value

ACTION: Will set the LED colour to the hex string passed in the message

Add widgets to the MQTT Dash app as shown in the below example

![](https://github.com/AshleydoRego/IoT-Lightbender/blob/main/Images/MQTT%20Dash%20Widget%20Settings.jpg)

Your Final App should look like this:

![](https://github.com/AshleydoRego/IoT-Lightbender/blob/main/Images/MQTT%20Dash.jpg)

## DEMO



https://user-images.githubusercontent.com/40831270/132221162-6bee0579-d2e3-4809-b7d0-613e4e27ee15.mp4




