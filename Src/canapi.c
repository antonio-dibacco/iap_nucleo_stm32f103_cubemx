/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"
#include "common.h"
#include "ymodem.h"
#include "string.h"
#include "menu.h"
#include "can.h"
#include "canapi.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CRC8_F       /* activate the CRC8 integrity */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* @note ATTENTION - please keep this variable 32bit alligned */
static uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];
static CanTxMsgTypeDef transmitMsg;
static CanRxMsgTypeDef receiveMsg;

/* Private function prototypes -----------------------------------------------*/
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte);
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size);
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Receive a packet from sender
 * @param  data
 * @param  length
 *     0: end of transmission
 *     2: abort by sender
 *    >0: packet length
 * @param  timeout
 * @retval HAL_OK: normally return
 *         HAL_BUSY: abort by user
 */
Command_TypeDef CAN_Listen_For_Command(Transfer_Session* pSession, uint32_t timeout)
{
	uint32_t crc;
	uint32_t received = 0;
	HAL_StatusTypeDef status = HAL_TIMEOUT;
	Command_TypeDef result = NO_CMD;

	uint8_t char1;
	int i = 0;

	hcan.pTxMsg = &transmitMsg;
	hcan.pRxMsg = &receiveMsg;

	// HELLO
	hcan.pTxMsg->DLC = 2;
	hcan.pTxMsg->Data[0] = READY_TO_START;


	for (i = 0; (i < 3) && (status != HAL_OK); i++)
	{
		hcan.pTxMsg->Data[1] = i;

		HAL_CAN_Transmit(&hcan, 500);
		status = HAL_CAN_Receive(&hcan, CAN_FIFO0, 3000);

	}


	if (status == HAL_OK) {
		if (hcan.pRxMsg->Data[0] == START_DOWNLOAD)
		{
			FLASH_If_Erase(APPLICATION_ADDRESS);

			hcan.pTxMsg->DLC = 1;
			hcan.pTxMsg->Data[0] = START_DOWNLOAD_OK;
			status = HAL_CAN_Transmit(&hcan, 300);
			return DOWNLOAD_CMD;
		}
		else if (hcan.pRxMsg->Data[0] == RESET_REQUEST)
		{
			return RESET_CMD;
		}
		else
		{
			return START_CMD;
		}
	}
	else
	{
		return NO_CMD;
	}
}

/**
 * @brief  Receive a packet from sender
 * @param  data
 * @param  length
 *     0: end of transmission
 *     2: abort by sender
 *    >0: packet length
 * @param  timeout
 * @retval HAL_OK: normally return
 *         HAL_BUSY: abort by user
 */
Frame_TypeDef CAN_Receive_Packet(uint8_t *p_data, uint32_t* len, uint32_t timeout) {
	uint32_t crc;
	uint32_t received = 0;
	uint32_t packet_length = 0;
	HAL_StatusTypeDef status = HAL_TIMEOUT;
	Frame_TypeDef result = BAD_PACKET;
	uint8_t char1;

	hcan.pTxMsg = &transmitMsg;
	hcan.pRxMsg = &receiveMsg;

	// Wait for either a start of data packet or control packet
	status = HAL_CAN_Receive(&hcan, CAN_FIFO0, 2*timeout);

	if (status == HAL_OK)
	{
		switch (hcan.pRxMsg->Data[0])
		{
			case START_OF_PACKET:
			{
				packet_length = *((uint32_t*) &hcan.pRxMsg->Data[1]);

				status = HAL_OK;
				while ((received < packet_length) && (status == HAL_OK))
				{
					status = HAL_CAN_Receive(&hcan, CAN_FIFO0, timeout);

					memcpy(p_data, hcan.pRxMsg->Data, hcan.pRxMsg->DLC);
					p_data += hcan.pRxMsg->DLC;
					received += hcan.pRxMsg->DLC;

				}

				if (received == packet_length)
				{
					status = HAL_CAN_Receive(&hcan, CAN_FIFO0, timeout);
					if (status == HAL_OK)
					{
						if (hcan.pRxMsg->Data[0] == END_OF_PACKET)
						{
							result = DATA;
							*len = packet_length;
							hcan.pTxMsg->DLC = 1;
							hcan.pTxMsg->Data[0] = END_OF_PACKET_OK;

							status = HAL_CAN_Transmit(&hcan, 500);

						}
						else
							result = BAD_PACKET;
					}
					else
					{
						result = BAD_PACKET;
					}
				}
				else
				{
					result = BAD_PACKET;
				}
			}
				break;
			default:
				result = BAD_PACKET;
				break;

		}
	}
	else
	{
		result = BAD_PACKET;
	}

	if (result == BAD_PACKET)
	{
		hcan.pTxMsg->DLC = 1;
		hcan.pTxMsg->Data[0] = BAD_PACKET;

		status = HAL_CAN_Transmit(&hcan, 500);
	}


	return result;
}




/*******************(C)COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
