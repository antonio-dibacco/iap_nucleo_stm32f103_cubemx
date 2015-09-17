# iap_nucleo_stm32f103_cubemx
In Application Programming for nucleo stm32f103rb using Cube MX and UART. 
Like the ST IAP it presents a menu where you can load a new firmware at APPLICATION_ADDRESS (0x08008000) 
in flash. 
This project is for AC6 studio and the initial processor configuration is done using Cube MX.

The blinky binary is a simple led blinking application that has a FLASH_ORIGIN set to APPLICATION_ADDRESS 
instead of 0x08000000 (real flash address). 
The VECT_TAB_OFFSET is set to (APPLICATION_ADDRESS-0x08000000) = 0x8000



