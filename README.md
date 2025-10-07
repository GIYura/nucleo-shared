### Brief

Contains SW modules shared accross projects.

### Structure

| SW module name  |       Description           |             Notes            |
|-----------------|-----------------------------|------------------------------|
| buffer          | Circular buffer             |                              |
| assert          | Custom assertion            |                              |
| delay           | Blocking delay              | Based on SysTick (s, ms, us) |
| event           | Event queue                 |                              |
| gpio            | GPIO diver                  |                              |
| hw-timer        | HW timer driver             | TODO                         |
| logger          | Serial logger               | UART1 (921600-8N1)           |
| mpu             | Memory Protection Unit      |                              |
| pwm             | PWM driver                  | TIM1 (refactor)              |
| spi             | SPI driver                  |                              |
| i2c             | I2C driver                  |                              |
| uart            | UART driver                 |                              |
| adxl345         | ADXL345 driver              |                              |
| utils           | Helper macros               |                              |
| freeRTOS-kernel | FreeRTOS kernel files       | FreeRTOS kernel V11.1.0      |

