/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : flash1.h
**     Project     : Bootloader_v2.0
**     Processor   : S32K144_100
**     Component   : flash
**     Version     : Component 1.0.0, Driver 01.00, CPU db: 3.00.000
**     Repository  : SDK_S32K14x_07
**     Compiler    : GNU C Compiler
**     Date/Time   : 2024-01-05, 21:22, # CodeGen: 1
**     Contents    :
**         FLASH_DRV_Init                  - status_t FLASH_DRV_Init(const flash_user_config_t * const pUserConf,...
**         FLASH_DRV_GetPFlashProtection   - void FLASH_DRV_GetPFlashProtection(uint32_t * protectStatus);
**         FLASH_DRV_SetPFlashProtection   - status_t FLASH_DRV_SetPFlashProtection(uint32_t protectStatus);
**         FLASH_DRV_GetSecurityState      - void FLASH_DRV_GetSecurityState(uint8_t * securityState);
**         FLASH_DRV_SecurityBypass        - status_t FLASH_DRV_SecurityBypass(const flash_ssd_config_t * pSSDConfig,const...
**         FLASH_DRV_EraseAllBlock         - status_t FLASH_DRV_EraseAllBlock(const flash_ssd_config_t * pSSDConfig);
**         FLASH_DRV_VerifyAllBlock        - status_t FLASH_DRV_VerifyAllBlock(const flash_ssd_config_t *...
**         FLASH_DRV_EraseSector           - status_t FLASH_DRV_EraseSector(const flash_ssd_config_t * pSSDConfig,uint32_t...
**         FLASH_DRV_VerifySection         - status_t FLASH_DRV_VerifySection(cont flash_ssd_config_t *...
**         FLASH_DRV_EraseSuspend          - void FLASH_DRV_EraseSuspend(void);
**         FLASH_DRV_EraseResume           - void FLASH_DRV_EraseResume(void);
**         FLASH_DRV_ReadOnce              - status_t FLASH_DRV_ReadOnce(const flash_ssd_config_t * pSSDConfig,uint8_t...
**         FLASH_DRV_ProgramOnce           - status_t FLASH_DRV_ProgramOnce(const flash_ssd_config_t * pSSDConfig,uint8_t...
**         FLASH_DRV_Program               - status_t FLASH_DRV_Program(const flash_ssd_config_t * pSSDConfig,uint32_t...
**         FLASH_DRV_ProgramCheck          - status_t FLASH_DRV_ProgramCheck(const flash_ssd_config_t *...
**         FLASH_DRV_CheckSum              - status_t FLASH_DRV_CheckSum(const flash_ssd_config_t * pSSDConfig,uint32_t...
**         FLASH_DRV_ProgramSection        - status_t FLASH_DRV_ProgramSection(const flash_ssd_config_t *...
**         FLASH_DRV_EraseBlock            - status_t FLASH_DRV_EraseBlock(const flash_ssd_config_t * pSSDConfig,uint32_t...
**         FLASH_DRV_VerifyBlock           - status_t FLASH_DRV_VerifyBlock(const flash_ssd_config_t * pSSDConfig,uint32_t...
**         FLASH_DRV_GetEERAMProtection    - status_t FLASH_DRV_GetEERAMProtection(uint8_t * protectStatus);
**         FLASH_DRV_SetEERAMProtection    - status_t FLASH_DRV_SetEERAMProtection(uint8_t protectStatus);
**         FLASH_DRV_SetFlexRamFunction    - status_t FLASH_DRV_SetFlexRamFunction(const flash_ssd_config_t * pSSDConfig,...
**         FLASH_DRV_EEEWrite              - status_t FLASH_DRV_EEEWrite(const flash_ssd_config_t * pSSDConfig,uint32_t...
**         FLASH_DRV_DEFlashPartition      - status_t FLASH_DRV_DEFlashPartition(const flash_ssd_config_t * pSSDConfig,...
**         FLASH_DRV_GetDFlashProtection   - status_t FLASH_DRV_GetDFlashProtection(const flash_ssd_config_t *...
**         FLASH_DRV_SetDFlashProtection   - status_t FLASH_DRV_SetDFlashProtection(const flash_ssd_config_t *...
**         FLASH_DRV_PFlashSwap            - status_t FLASH_DRV_PFlashSwap(const flash_ssd_config_t * pSSDConfig,uint32_t...
**         FLASH_DRV_PFlashSwapCtl         - status_t FLASH_DRV_PFlashSwapCtl(const flash_ssd_config_t *...
**         FLASH_DRV_EraseAllBlockUnsecure - status_t FLASH_DRV_EraseAllBlockUnsecure(const flash_ssd_config_t * pSSDConfig);
**
**     Copyright 1997 - 2015 Freescale Semiconductor, Inc.
**     Copyright 2016-2017 NXP
**     All Rights Reserved.
**     
**     THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
**     IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
**     OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
**     IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
**     INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
**     SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
**     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
**     STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
**     IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
**     THE POSSIBILITY OF SUCH DAMAGE.
** ###################################################################*/
/*!
** @file flash1.h
** @version 01.00
*/
/*!
**  @addtogroup flash1_module flash1 module documentation
**  @{
*/
#ifndef flash1_H
#define flash1_H

/* MODULE flash1.
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.5, Global macro not referenced.
 * The global macro will be used in function call of the module.
 */

/* Include inherited beans */
#include "flash_driver.h"

/*! @brief User configuration structure 0 */
extern const flash_user_config_t flash1_InitConfig0;


#endif
/* ifndef flash1_H */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.1 [05.21]
**     for the Freescale S32K series of microcontrollers.
**
** ###################################################################
*/
