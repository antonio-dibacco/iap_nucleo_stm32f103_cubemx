/**
 ******************************************************************************
 * @file    IAP_Main/Src/menu.c
 * @author  MCD Application Team
 * @version 1.0.0
 * @date    8-April-2015
 * @brief   This file provides the software which contains the main menu routine.
 *          The main menu gives the options of:
 *             - downloading a new binary file,
 *             - uploading internal flash memory,
 *             - executing the binary file already loaded
 *             - configuring the write protection of the Flash sectors where the
 *               user loads his binary file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/** @addtogroup STM32F1xx_IAP
 * @{
 */

/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"
#include "menu.h"
#include "ymodem.h"
#include "usart.h"
#include "can.h"
#include "canapi.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

uint32_t FlashProtection = 0;
uint8_t aFileName[FILE_NAME_LENGTH];
extern pFunction JumpToApplication;
extern uint32_t JumpAddress;

uint8_t aPacketData[2048];


/* Private function prototypes -----------------------------------------------*/
void SerialDownload(void);
void SerialUpload(void);

/* Private functions ---------------------------------------------------------*/



/**
 * @brief  Download a file via serial port
 * @param  None
 * @retval None
 */
void SerialDownload(void)
{
	uint8_t number[11] = {0};
	uint32_t size = 0;
	COM_StatusTypeDef result;

	Serial_PutString("Waiting for the file to be sent ... (press 'a' to abort)\n\r");
	result = Ymodem_Receive( &size );
	if (result == COM_OK)
	{
		Serial_PutString("\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: ");
		Serial_PutString(aFileName);
		Int2Str(number, size);
		Serial_PutString("\n\r Size: ");
		Serial_PutString(number);
		Serial_PutString(" Bytes\r\n");
		Serial_PutString("-------------------\n");
	}
	else if (result == COM_LIMIT)
	{
		Serial_PutString("\n\n\rThe image size is higher than the allowed space memory!\n\r");
	}
	else if (result == COM_DATA)
	{
		Serial_PutString("\n\n\rVerification failed!\n\r");
	}
	else if (result == COM_ABORT)
	{
		Serial_PutString("\r\n\nAborted by user.\n\r");
	}
	else
	{
		Serial_PutString("\n\rFailed to receive the file!\n\r");
	}
}


/**
 * @brief  Upload a file via serial port.
 * @param  None
 * @retval None
 */
void Start_Application(void)
{
	/* Test if user code is programmed starting from address "APPLICATION_ADDRESS" */
	if (((*(__IO uint32_t*) APPLICATION_ADDRESS ) & 0x2FFE0000) == 0x20000000) {
		/* Jump to user application */
		JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
		JumpToApplication = (pFunction) JumpAddress;
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
		JumpToApplication();
	}
	else {
		NVIC_SystemReset();
	}
}

/**
 * @brief  Upload a file via serial port.
 * @param  None
 * @retval None
 */
void SerialUpload(void)
{
	uint8_t status = 0;

	Serial_PutString("\n\n\rSelect Receive File\n\r");

	HAL_UART_Receive(&huart1, &status, 1, RX_TIMEOUT);
	if ( status == CRC16)
	{
		/* Transmit the flash image through ymodem protocol */
		status = Ymodem_Transmit((uint8_t*)APPLICATION_ADDRESS, (const uint8_t*)"UploadedFlashImage.bin", USER_FLASH_SIZE);

		if (status != 0)
		{
			Serial_PutString("\n\rError Occurred while Transmitting File\n\r");
		}
		else
		{
			Serial_PutString("\n\rFile uploaded successfully \n\r");
		}
	}
}

/**
 * @brief
 * @param  None
 * @retval None
 */
void Load_Firmware_CAN(void)
{
	int len = 0;
	uint32_t flash_destination, ram_source;

	CAN_FilterConfTypeDef CAN_FilterInitStructure;

	CAN_FilterInitStructure.FilterNumber = 0;
	CAN_FilterInitStructure.FilterMode = CAN_FILTERMODE_IDMASK;
	CAN_FilterInitStructure.FilterScale = CAN_FILTERSCALE_16BIT;
	CAN_FilterInitStructure.FilterIdHigh = 0x0000;
	CAN_FilterInitStructure.FilterIdLow = 0x0000;
	CAN_FilterInitStructure.FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	CAN_FilterInitStructure.FilterActivation = ENABLE;
	HAL_StatusTypeDef res = HAL_CAN_ConfigFilter(&hcan, &CAN_FilterInitStructure);

	HAL_CAN_WakeUp(&hcan);

//	CAN_Transmit_Packet(aPacketData,8,1000);

	PacketStatus_TypeDef result = CAN_Receive_Packet(aPacketData, &len, 4000);

	if (result == PACKET_BAD)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	}

	if (result == PACKET_OK)
	{
		if (aPacketData[0] == DOWNLOAD_CMD)
		{
			uint32_t file_length = *((uint32_t*) &aPacketData[1]);
			flash_destination = APPLICATION_ADDRESS;
			FLASH_If_Erase(APPLICATION_ADDRESS);

			do {
				int packet_length = 0;

				result = CAN_Receive_Packet(aPacketData, &packet_length, 3000);

				if ((result == PACKET_OK)) {
					ram_source = (uint32_t) &aPacketData[0];

					/* Write received data in Flash */
					if (FLASH_If_Write(flash_destination, (uint32_t*) ram_source, packet_length/4) == FLASHIF_OK)
					{
						flash_destination += packet_length;
						file_length -= packet_length;

					}
					else /* An error occurred while writing to Flash memory */
					{
						// Reset
						NVIC_SystemReset();
					}
				}

			} while (file_length > 0 );

			Start_Application();

		}
		else if (aPacketData[0]  == RESET_CMD)
		{
			// Didn't receive any message, just startup
			NVIC_SystemReset();
		}
		else
		{
			// Didn't receive any message, just startup
			Start_Application();
		}
	}
	if(result == RX_TIMEOUT){
		Start_Application();
	}


}

/**
 * @brief  Display the Main Menu on HyperTerminal
 * @param  None
 * @retval None
 */
void Load_Firmware_Serial(void)
{
	int i = 0;
	uint8_t key = 0;

	Serial_PutString("\r\n======================================================================");
	Serial_PutString("\r\n=              (C) COPYRIGHT 2015 YOUNCTA                            =");
	Serial_PutString("\r\n=                                                                    =");
	Serial_PutString("\r\n=  STM32F1xx In-Application Programming Application  (Version 1.0.0) =");
	Serial_PutString("\r\n=                                                                    =");
	Serial_PutString("\r\n======================================================================");
	Serial_PutString("\r\n\r\n");

	/* Test if any sector of Flash memory where user application will be loaded is write protected */
	FlashProtection = FLASH_If_GetWriteProtectionStatus();

	while (1)
	{

		Serial_PutString("\r\n=================== Main Menu ============================\r\n\n");
		Serial_PutString("  Download image to the internal Flash ----------------- 1\r\n\n");
		Serial_PutString("  Upload image from the internal Flash ----------------- 2\r\n\n");
		Serial_PutString("  Execute the loaded application ----------------------- 3\r\n\n");


		if(FlashProtection != FLASHIF_PROTECTION_NONE)
		{
			Serial_PutString("  Disable the write protection ------------------------- 4\r\n\n");
		}
		else
		{
			Serial_PutString("  Enable the write protection -------------------------- 4\r\n\n");
		}
		Serial_PutString("==========================================================\r\n\n");

		/* Clean the input path */
		__HAL_UART_FLUSH_DRREGISTER(&huart1);

		/* Receive key */
		HAL_StatusTypeDef status = HAL_TIMEOUT;
		while ((i++ < 3) && (status != HAL_OK)) {
			status = HAL_UART_Receive(&huart1, &key, 1, 2000);
		}

		if (status == HAL_OK) {
			switch (key)
			{
			case '1' :
				/* Download user application in the Flash */
				SerialDownload();
				break;
			case '2' :
				/* Upload user application from the Flash */
				/* not needed */
				SerialUpload();
				break;
			case '3' :
				Serial_PutString("Starting ......\r\n\n");
				Start_Application();
				break;
			case '4' :
				if (FlashProtection != FLASHIF_PROTECTION_NONE)
				{
					/* Disable the write protection */
					if (FLASH_If_WriteProtectionConfig(FLASHIF_WRP_DISABLE) == FLASHIF_OK)
					{
						Serial_PutString("Write Protection disabled...\r\n");
						Serial_PutString("System will now restart...\r\n");
						/* Launch the option byte loading */
						HAL_FLASH_OB_Launch();
					}
					else
					{
						Serial_PutString("Error: Flash write un-protection failed...\r\n");
					}
				}
				else
				{
					if (FLASH_If_WriteProtectionConfig(FLASHIF_WRP_ENABLE) == FLASHIF_OK)
					{
						Serial_PutString("Write Protection enabled...\r\n");
						Serial_PutString("System will now restart...\r\n");
						/* Launch the option byte loading */
						HAL_FLASH_OB_Launch();
					}
					else
					{
						Serial_PutString("Error: Flash write protection failed...\r\n");
					}
				}
				break;
			default:
				Serial_PutString("Invalid Number ! ==> The number should be either 1, 2, 3 or 4\r");
				break;
			}
		}
		else {
			// Didn't receive any message, just startup
			Serial_PutString("Starting due to timeout ......\r\n\n");
			Start_Application();
		}
	}
}

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
