/**
  ******************************************************************************
  * @file    IAP_Main/Inc/ymodem.h 
  * @author  MCD Application Team
  * @version 1.0.0
  * @date    8-April-2015
  * @brief   This file provides all the software function headers of the ymodem.c 
  *          file.
  ******************************************************************************
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FIRMWARE_DOWNLOAD_H_
#define __FIRMWARE_DOWNLOAD_H_

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/


/**
  * @}
  */

typedef enum
{
	PACKET_64      			=   64,
	PACKET_128      		=  128,
	PACKET_256      		=  256,
	PACKET_512      		=  512,
	PACKET_1024      		= 1024,
	PACKET_2048       		= 2048,
	PACKET_4096       		= 4096

} Packet_SizeTypeDef;


typedef struct
{
	uint32_t	size;
	uint32_t	packet_size;
} Transfer_Session;

typedef enum
{
	READY_TO_START    = 1,
	START_DOWNLOAD       ,
	START_DOWNLOAD_OK    ,
	START_OF_PACKET		 ,
	DATA		         ,
	END_OF_PACKET		 ,
	END_OF_PACKET_OK  	 ,
	CONTROL				 ,
	RESET_REQUEST  		 ,
	BAD_PACKET 		  = 255
} Frame_TypeDef;


typedef enum
{
	NO_CMD				= 0,
	DOWNLOAD_CMD		= 1,
	RESET_CMD			= 2,
	START_CMD			= 3,
	VERSION_CMD			= 4
} Command_TypeDef;

HAL_StatusTypeDef CAN_Listen_For_Transfer(Transfer_Session* pSession, uint32_t timeout);
Frame_TypeDef CAN_Receive_Packet(uint8_t *p_data, uint32_t* len, uint32_t timeout);


#endif  /* __FIRMWARE_DOWNLOAD_H_ */

/*******************(C)COPYRIGHT STMicroelectronics ********END OF FILE********/
