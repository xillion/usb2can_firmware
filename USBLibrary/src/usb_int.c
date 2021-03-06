/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : usb_int.c
* Author             : MCD Application Team
* Date First Issued  : 10/27/2003 : V1.0
* Description        : Endpoint CTR (Low and High) interrupt's service routines
********************************************************************************
* History:
* 09/18/2006 : V3.0
* 09/01/2006 : V2.0
* 10/27/2003 : V1.0
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "stdio.h"  // GS
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u16 SaveRState;
u16 SaveTState;
/* Extern variables ----------------------------------------------------------*/
extern void (*pEpInt_IN[15])(void);    /*  Handles IN  interrupts   */
extern void (*pEpInt_OUT[15])(void);   /*  Handles OUT interrupts   */
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : CTR_LP
* Description    : Low Endpoint Correct Transfer interrupt's service routine
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CTR_LP(void)
{
 u32 wEPVal;
 	/* stay in loop while pending ints */
	while(((wIstr = _GetISTR()) & ISTR_CTR)!= 0)
	{
	    _SetISTR((u16)CLR_CTR); /* clear CTR flag */
                                    
		/* extract highest priority endpoint number */
		EPindex = (u8)(wIstr & ISTR_EP_ID);
                
                //
                //("L%d",EPindex); // GS
                
		if(EPindex == 0)
		{
                  
                  //printf(" %d",EPindex); 
                  
                        /* Decode and service control endpoint interrupt */
                        /* calling related service routine */
                        /* (Setup0_Process, In0_Process, Out0_Process) */

			/* save RX & TX status */
			/* and set both to NAK */
			SaveRState = _GetEPRxStatus(ENDP0);
			SaveTState = _GetEPTxStatus(ENDP0);
			_SetEPRxStatus(ENDP0, EP_RX_NAK);
			_SetEPTxStatus(ENDP0, EP_TX_NAK);
                      

			/* DIR bit = origin of the interrupt */

			if((wIstr & ISTR_DIR) == 0) // GS **** IN tranzakcija ***
			{/* DIR = 0	*/
			
				/* DIR = 0      => IN  int */
				/* DIR = 0 implies that (EP_CTR_TX = 1) always  */
			
				
				_ClearEP_CTR_TX(ENDP0);                                                                
                                
			        In0_Process();
				
				/* check if SETUP arrived during IN processing */
				wEPVal = _GetENDPOINT(ENDP0);                                
                                
                                
				if((wEPVal & (EP_CTR_RX|EP_SETUP)) != 0)
				{
			         _ClearEP_CTR_RX(ENDP0); /* SETUP bit kept frozen while CTR_RX = 1 */
				 Setup0_Process();
				}

			}/* DIR = 0	*/
			else
			{/* DIR = 1 */ // GS *** OUT tranzakcija ***

			
				/* DIR = 1 & CTR_RX       => SETUP or OUT int */
				/* DIR = 1 & (CTR_TX | CTR_RX) => 2 int pending */
			
				wEPVal = _GetENDPOINT(ENDP0);
				if((wEPVal & EP_CTR_TX) != 0)
				{
					_ClearEP_CTR_TX(ENDP0);
					In0_Process();
				}
				
				
                               	if((wEPVal &EP_SETUP) != 0)
				{
					_ClearEP_CTR_RX(ENDP0); /* SETUP bit kept frozen while CTR_RX = 1 */
					Setup0_Process();
				}
                                else if((wEPVal & EP_CTR_RX) != 0)
				{
					_ClearEP_CTR_RX(ENDP0);
					Out0_Process();
				}

			}/* DIR = 1 */

			/* before terminate set Tx & Rx status */
			_SetEPRxStatus(ENDP0, SaveRState);
			_SetEPTxStatus(ENDP0, SaveTState);
		}/* if(EPindex == 0) */
		else
		{  /* Decode and service non control endpoints interrupt  */

			/* process related endpoint register */
			wEPVal = _GetENDPOINT(EPindex);
			if((wEPVal & EP_CTR_RX) != 0)
			{
					/* clear int flag */
					_ClearEP_CTR_RX(EPindex);
					
					/* call OUT service function */
                                        //printf(" %d",EPindex); 
					(*pEpInt_OUT[EPindex-1])();

			} /* if((wEPVal & EP_CTR_RX) */
			if((wEPVal & EP_CTR_TX) != 0)
			{
					/* clear int flag */
					_ClearEP_CTR_TX(EPindex);
					
					/* call IN service function */
                                        //printf(" %d",EPindex); 
					(*pEpInt_IN[EPindex-1])();
					
					
			} /* if((wEPVal & EP_CTR_TX) != 0) */

			
		}/* if(EPindex == 0) else	*/

	}/* while(...)	*/
} /* CTR_LP */


/*******************************************************************************
* Function Name  : CTR_HP
* Description    : High Endpoint Correct Transfer interrupt's service routine
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CTR_HP(void)
{
 u32 wEPVal;
 	
            while(((wIstr = _GetISTR()) & ISTR_CTR)!= 0)
	      {
		_SetISTR((u16)CLR_CTR); /* clear CTR flag */
		/* extract highest priority endpoint number */
		EPindex = (u8)(wIstr & ISTR_EP_ID);
                
                //printf("H%d",EPindex); // GS
                
		/* process related endpoint register */
			wEPVal = _GetENDPOINT(EPindex);
			if((wEPVal & EP_CTR_RX) != 0)
			{
					/* clear int flag */
					_ClearEP_CTR_RX(EPindex);
					
					/* call OUT service function */
                                        //printf(" %d",EPindex); 
					(*pEpInt_OUT[EPindex-1])();

			} /* if((wEPVal & EP_CTR_RX) */
			if((wEPVal & EP_CTR_TX) != 0)
			{
					/* clear int flag */
					_ClearEP_CTR_TX(EPindex);
					
					/* call IN service function */
                                        //printf(" %d",EPindex); 
					(*pEpInt_IN[EPindex-1])();
					
					
			} /* if((wEPVal & EP_CTR_TX) != 0) */

	      }/* while(...)	*/
} /* CTR_HP */

/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/

