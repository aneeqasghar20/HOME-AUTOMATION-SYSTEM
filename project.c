#include <stdio.h>
#include <stdint.h>
#include "C:\ti\TivaWare_C_Series-2.1.4.178\inc\tm4c123gh6pm.h"
#include <string.h>

float result_temperature;
float result_smoke;
char x;
volatile int a;

void EnableInterrupts(void);
void UART0_init(void);
char UART0Rx(void);
void UART0Tx(char c);
void sendString(char*ptr);
void adc0_seq3_init();
void ADC0Seq3_Handler();
void adc1_seq3_init();
void ADC1Seq3_Handler();
void portF_init();
void intinit(void);
void delayms(int n);
void PWM_init();
void GPIOPortF_Handler(void);
void UART0_Handler();


void EnableInterrupts(void)
{
   _asm("   cpsie i\n") ;
}
void UART0_init(void)
{
     SYSCTL_RCGCUART_R   |=  1;     /*   provide clock to UART0  */
     SYSCTL_RCGCGPIO_R  |=  0x01;    /*  enable clock to GPIOA   */

     /*  UART0 initialization  */
     UART0_CTL_R  =  0;                 /* disable UART0  */
     UART0_IBRD_R =  104;           /*  16MHz/16=1MHz,  1MHz/104=9600 baud rate */
     UART0_FBRD_R =  11;           /*  fraction part   */
     UART0_CC_R  	=   0;                /*  use system clock  */
     UART0_LCRH_R = 0x60;          /*  0x08-bit, no parity, 1-stop bit, no FIFO  */
     UART0_CTL_R  =  0x301;       /*  enable UART0, TXE , RXE   */

      /*  UART0 TX0 and RX0 use PA0 and PA1. Set them up.  */
      GPIO_PORTA_DEN_R  =  0x03;       /*  Make PAO and PAI as digital */
      GPIO_PORTA_AMSEL_R  &=  ~0x03;   //  No analog on PA1-0
      GPIO_PORTA_AFSEL_R  =  0x03;   /*  Use PA0,PA1 alternate fucntion  */
      GPIO_PORTA_PCTL_R  =  0x11;    /* configure PA0 and PA1 for UART  */


      NVIC_EN0_R  =0x20;
      NVIC_PRI1_R=0<<5;
      UART0_IM_R=0X10;
      EnableInterrupts();
}
char UART0Rx(void)
{
		while ((UART0_FR_R&0x0010)!=0) {} ;
		return ( (char) (UART0_DR_R&0xFF) ) ;
}
 void UART0Tx(char  c)
{
		while((UART0_FR_R &0x20)!=0){};
    UART0_DR_R  =  c;
}
void sendString (char*ptr)
{
	while (*ptr != 0)/* if not end of string */
	{
	UART0Tx(*ptr++); /* send the character through UART0 */
	}
}


void adc0_seq3_init() {
          /* enable clocks  */
       SYSCTL_RCGCGPIO_R|=0x10;
       SYSCTL_RCGCADC_R |=  1;       /* enable clock to ADC0 */
       
       GPIO_PORTE_AFSEL_R  |=0x08;
       GPIO_PORTE_DEN_R  &=~0x08;
       GPIO_PORTE_AMSEL_R  |=0x08;

         /* initialize ADC0  */
       ADC0_ACTSS_R   &=  ~0x08;         /* disable SS3 during configuration */
       ADC0_EMUX_R   &=  ~0xF000;
    
       ADC0_SSMUX3_R  =  0x00;          /* get input from channel  0  */  // AINO
       ADC0_SSCTL3_R   |=   0x06;        /* take chip result, set flag at 1st sample  */

       NVIC_EN0_R   |=0X00020000;
       ADC0_IM_R  |=  0x08;
       ADC0_ACTSS_R  |=  0x08;           /*enable ADC0  sequencer  3  */
       EnableInterrupts ();
}          
void ADC0Seq3_Handler()
{
	result_temperature = ADC0_SSFIFO3_R * 330/4096;
	ADC0_ISC_R = 0x08;            /* clear completion flag*/
}
void adc1_seq3_init()
{
	SYSCTL_RCGCGPIO_R |= 0x10; /* enable clock to PORTE */
	SYSCTL_RCGCADC_R |= 2; /* Enable Clock to ADC0*/
	GPIO_PORTE_AFSEL_R |= 0x04;
	GPIO_PORTE_DEN_R &=~ 0x04;
	/* INITIALIZE ADC1 */
	ADC1_ACTSS_R &=~ 0x08;  /* Disable SS3 during configuration */
	ADC1_ACTSS_R &=~ 0xF000;

	ADC1_SSMUX3_R = 0x01;   /* Get input from channel 1 AIN1 */
	ADC1_SSCTL3_R = 0x06;   /* Take chip result, set flag at 1st sample */

	NVIC_EN1_R |= 0x00080000;
	ADC1_IM_R |= 0x08;
	ADC1_ACTSS_R |= 0x08; /* Enable ADC1 SS3 */

	EnableInterrupts();
}

void ADC1Seq3_Handler(){
	result_smoke = ADC1_SSFIFO3_R * 330/4096.0;
	ADC1_ISC_R = 0x08; /* Clear Completion Flag */
}

void portF_init()
{
	SYSCTL_RCGCGPIO_R |= 0x20; /* Enable Clock of PORTF */
	GPIO_PORTF_LOCK_R = 0x4C4F4348;
	GPIO_PORTF_CR_R = 0x10; /* Assigns pin to SSI2 */
	GPIO_PORTF_DEN_R = 0x1F;
	GPIO_PORTF_DIR_R = 0x0E; /* PF1,PF2, PF3 INPUT */
	GPIO_PORTF_PUR_R =0x10; /* PF4 input*/
}

void intinit(void){
	GPIO_PORTF_IS_R &=~ 0x10;
	GPIO_PORTF_IEV_R &=~0x10;
	GPIO_PORTF_ICR_R |= 0x10;
	NVIC_EN0_R |= 0x40000000;
	NVIC_PRI7_R= 1<<21;
	GPIO_PORTF_IM_R = 0x10;
	EnableInterrupts();
}

void delayms(int n)
{
	int j;
	for (a=0;a<n;a++)
	{
		for (j=0;j<3180;j++){}
	}
}

void PWM_init()
{
	SYSCTL_RCGCPWM_R |= 1;  //ENABLE CLOCK FROM PWM0
	SYSCTL_RCGCGPIO_R |= 0x04; //ENABLE PORTC
	SYSCTL_RCC_R  &=~ 0x00100000; //NO PRE-DIVIDE FOR PWM CLOCK
	GPIO_PORTC_AFSEL_R = 0x10; // ENABLE ALTERNATE FUNCTION FOR PC4
	GPIO_PORTC_PCTL_R   &=~ 0x000F0000;
	GPIO_PORTC_PCTL_R    |=  0x00040000; /* ENABLE PWM FOR PC4 */
	GPIO_PORTC_DEN_R |= 0x10;

	PWM0_3_CTL_R = 0;
	PWM0_3_GENA_R = 0x0000008C;
	PWM0_3_CTL_R = 1;
	PWM0_ENABLE_R = 0x40;
	delayms(5000);
}

void GPIOPortF_Handler(void){
	volatile int readback;
	char* str = "INTRUDER ALERT : Press A to turn off the alarm";
	sendString(str);
	str="\n";
	
	PWM_init();
	PWM0_3_LOAD_R = 16000;
	while(GPIO_PORTC_MIS_R != 0)
	{
		delayms(1000);
		PWM0_ENABLE_R =0x00;
		GPIO_PORTF_ICR_R = 0x11;
		readback = GPIO_PORTF_ICR_R;
	}
}
void UART0_Handler(){
	volatile int readback;
	char c=UART0_DR_R;
	PWM0_ENABLE_R =0x00;
	a=1000;
	GPIO_PORTF_ICR_R = 0x10;
	readback = GPIO_PORTF_ICR_R;
}

int main(){

	portF_init();
	intinit();
	UART0_init();
	adc0_seq3_init();
	adc1_seq3_init();

	while(1)
	{
		char buffer[16];
		char *str="Room temperature is: ";
		char * degree ="Degree Celsius";
		char * str1 ="and the Smoke level is:  ";

		ADC0_PSSI_R |= 0x08;    /* Start a conversion sequence 3 */

		result_temperature = ADC0_SSFIFO3_R * 330/4096;
		/* clear completion flag */

		ADC1_PSSI_R |= 0x08;  /* START A CONVERSION SEQUENCE 3 */
		result_smoke = ADC1_SSFIFO3_R * 330/4096;
		/* clear completion flag */

		sendString(str);
		sprintf(buffer, "%f\r\n", (result_temperature-32) * 5/9 );
		sendString(buffer);
		sendString(str1);
		sprintf(buffer, "%f\r\n" , result_smoke);
		sendString(buffer);
		delayms(1000);

		if( ((result_temperature-32)* 5/9 ) > 30 ){
			PWM_init();
			PWM0_3_LOAD_R =16000;
			while(GPIO_PORTC_MIS_R != 0){
				PWM0_3_CMPA_R = 8000;
			}
			delayms(1000);
			PWM0_ENABLE_R = 0x00;
		}

		else if (result_smoke >200 )
		{
			PWM_init();
			PWM0_3_LOAD_R =16000;
			while(GPIO_PORTC_MIS_R != 0){
				PWM0_3_CMPA_R = 8000;
			}
			delayms(1000);
			PWM0_ENABLE_R = 0x00;
		}
	}
	return 0;
}