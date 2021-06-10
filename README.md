# ATmega328p-based Speed Trap
- Final project for [USC's EE109](https://bytes.usc.edu/files/ee109/documents/EE109_Syllabus.pdf)

## Functionality
- [Samples on-board adc](adc.c) to measure the speed of objects passing through neighboring photoresistors
- Compares speeds against a speed limit set by a [rotary encoder](encoder.c)
- Transfers speeds [over serial](serial.c) and displays them using [lcd drivers](lcd.c)
- Uses [timers to precisely actuate a microphone](timer.c) if the speed limit is breached
