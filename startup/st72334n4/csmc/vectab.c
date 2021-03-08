void EEPROM_IRQHandler();
void SCI_IRQHandler();
void TIMERB_IRQHandler();
void TIMERA_IRQHandler();
void SPI_IRQHandler();
void EXTI3_IRQHandler();
void EXTI2_IRQHandler();
void EXTI1_IRQHandler();
void EXTI0_IRQHandler();
void TRAP_IRQHandler();

void _stext(); // startup routine

#pragma section const { vector }

void (* const _vectab[16])() =
{
/*-16 */  0, //PWM_IRQHandler, // ART
/*-15 */  0, //I2C_IRQHandler,
/*-14 */  EEPROM_IRQHandler,
/*-13 */  SCI_IRQHandler,
/*-12 */  TIMERB_IRQHandler,
/*-11 */  TIMERA_IRQHandler,
/*-10 */  SPI_IRQHandler,
/* -9 */  0, //CAN_IRQHandler,
/* -8 */  EXTI3_IRQHandler,
/* -7 */  EXTI2_IRQHandler,
/* -6 */  EXTI1_IRQHandler,
/* -5 */  EXTI0_IRQHandler,
/* -4 */  0, //RTC_IRQHandler, // MCC
/* -3 */  0, //TLI_IRQHandler,
/* -2 */  TRAP_IRQHandler,
/* -1 */  _stext,
};
