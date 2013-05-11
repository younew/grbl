/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "config.h"
#include "limits.h"
#include "protocol.h"
/**
  * @brief  This function handles EXTI15_10
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void) 
{
  /*
  if(EXTI_GetITStatus(ETH_LINK_EXTI_LINE) != RESET)
  {
    Eth_Link_ITHandler(DP83848_PHY_ADDRESS);
    //Clear interrupt pending bit //
    EXTI_ClearITPendingBit(ETH_LINK_EXTI_LINE);
  }*/
#ifdef DEVELOP_LIMIT_ORIGN
  if(EXTI_GetITStatus(LIMITyo_EXTI_LINE) != RESET)
  {
    //if(IsLimitYo())
    //  gStatusLimitY |= LIMIT_Yo;
    //else
    //  gStatusLimit &= (~LIMIT_Yo);
    EXTI_ClearITPendingBit(LIMITyo_EXTI_LINE);
  }
#endif
}
/**
  * @brief  This function handles EXTI9_5
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void) 
{//------------------------------IN 限位------------------------------
  if(EXTI_GetITStatus(LIMITx_EXTI_LINE) != RESET)
  {
    IO_IN_ISR();
    EXTI_ClearITPendingBit(LIMITx_EXTI_LINE);
  }
}
void EXTI2_IRQHandler(void) 
{//------------------------------X 轴负限位------------------------------
  if(EXTI_GetITStatus(LIMITxn_EXTI_LINE) != RESET)
  {
    if(IsLimitXn())
    {
      g_limit |= LIMITxn_PIN;
      LimitISR();
    }
    else
      g_limit &= (~LIMITxn_PIN);
    
    IO_IN_ISR();
    EXTI_ClearITPendingBit(LIMITxn_EXTI_LINE);
  }
}
void EXTI3_IRQHandler(void) 
{//------------------------------X 轴正限位------------------------------
  if(EXTI_GetITStatus(LIMITx_EXTI_LINE) != RESET)
  {
    if(IsLimitX())
    {
      g_limit |= LIMITx_PIN;
      LimitISR();
    }
    else
      g_limit &= ~LIMITx_PIN;
    IO_IN_ISR();
    EXTI_ClearITPendingBit(LIMITx_EXTI_LINE);
  }
}
void EXTI4_IRQHandler(void) 
{//------------------------------X 轴原点限位------------------------------
  if(EXTI_GetITStatus(LIMITxo_EXTI_LINE) != RESET)
  {
#ifdef DEVELOP_LIMIT_ORIGN
    if(IsLimitXo())
    {
      g_limit |= LIMITxo_PIN;
    }
    else
      g_limit &= (~LIMITxo_PIN);
#endif
    EXTI_ClearITPendingBit(LIMITxo_EXTI_LINE);
  }
}
//------------------------------y轴 + - 限位------------------------------
void EXTI9_5_IRQHandler(void) 
{
  if(EXTI_GetITStatus(LIMITy_EXTI_LINE) != RESET)
  {
    if(IsLimitY())
    {
      g_limit |= LIMITyn_PIN;
      LimitISR();
    }
    else
      g_limit &= (~LIMITyn_PIN);
    EXTI_ClearITPendingBit(LIMITy_EXTI_LINE);
  }
  if(EXTI_GetITStatus(LIMITyn_EXTI_LINE) != RESET)
  {
    if(IsLimitYn())
    {
      g_limit |= LIMITy_PIN;
      LimitISR();
    }
    else
      g_limit &= (~LIMITy_PIN);
    EXTI_ClearITPendingBit(LIMITyn_EXTI_LINE);
  }
}
/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}
