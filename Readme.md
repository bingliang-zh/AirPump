# Air pump project

Write an Arduino project cpp file that met the purpose.

## Materials

Arduino Uno
Air pressure sensor: Linear 0-5V 0-1.6MPa
Potentiometer
Relay + Air pump motor
Switch button (Start pumping, stop pumping)
Display (IIC OLED): show current sensor's reads (Bar and PSI), and target pressure.

## How it works

When powered:
Screen shows the sensor's reads in both Bar and PSI on the first line, and target pressure on the second line with Bar and PSI too.
The sensor's reads is linear which means, 0V means 0MPa, and 5V means 1.6MPa. 1 MPa is 10 Bar, and 1 Bar is 14.5038 PSI.
The target pressure are from the potentiometer. It ranges from 0V to 5V the same way as the sensor. The controller will read the potentiometer's read and transform to the target pressure unit in Bar and PSI, then display it on the OLED display.
Switch button is default to stop state, if toggled, it will pump the air to the target pressure. If toggled again, it will stop the pumping and back to stop state.
Long press the switch button will enter the keep pressure mode. It will pump the air to the target pressure, and keep the target pressure to the target pressure if it drop 0.1 Bar.

## Button actions

There are two internal states:

- Standby / Holding / Pumping state
- One time / Persist mode

Long press the button will switch between One time and Persist mode.

Short press the button:

- If currently in Standby state, switch to Pumping state
- If currently not in Standby state, switch to Standby state

In One time mode:
If the sensor reads the air pressure reaches the target pressure, it will switch pumping state back to standby state.

In Persist mode:
If the sensor reads the air pressure reaches the target pressure, it will switch to holding state. If the sensor reads are less then 0.1 bar of the target pressure, it will switch to pumping state.

## When pumping

The target pressure can only be changed in standby mode.
