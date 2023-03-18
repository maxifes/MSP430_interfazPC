#include <msp430.h> 
#include <stdint.h>


//Git branch for development

//Configura UART
//- 8 bits de datos.
//- Sin bit de paridad.
//- 1 bit de stop.
//- 9600 baudrate.

#define SIZE_Data 100

uint8_t data[100];
unsigned int i = 0; //Contador de interupciones de recepción

void eUSCIA0_UART_send(int data_Tx){
    while((UCA0STATW & UCBUSY) == UCBUSY){}
    UCA0TXBUF = data_Tx; //Dato a enviar (pag.791) manual slau367p.pdf
}

void P4_PushButton_Init(){
    P4DIR &= ~BIT5; //habilita pin P4.2 como entrada digital
    P4REN |= BIT5; //Habilita resistencia de pull up o pulldown
    P4OUT |= BIT5; //Lo configura como pull up  //Logica positiva
    P4IES &= ~BIT5; //la bandera de interrupcion se activa con flanco positivo.
    P4IE |= BIT5;   //Activa la interrupciï¿½n en P4.2
    P4IFG = 0;
    _enable_interrupt();
}

void MSP430_Clk_Config(){
    CSCTL0_H = CSKEY >> 8;
    CSCTL1 = DCOFSEL_0 | DCORSEL;
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;
    CSCTL0_H = 0;
}

void eUSCIA1__UART_Init(){
    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 |= UCSSEL__SMCLK;
    UCA0BRW |= 6;
    UCA0MCTLW |= 0x20<<8 | UCOS16 | 8<<4;

    //Para 115200 bauds
    //bit rate
    //UCA0BRW = 8;
    //UCA0MCTLW |= 0xD600;


    P2SEL0 &= ~(BIT0 | BIT1);                   //P2SEL0.x = 0
    P2SEL1 |= BIT0 | BIT1;                      //P2SEL1.x = 1; Selecciona la función de UART en P2.1 y P2.0
    PM5CTL0 &= ~LOCKLPM5;

    UCA0CTLW0 &= ~UCSWRST;

    UCA0IE |= UCRXIE;                           //Habilita interrupción de recepción
                      //Habilita la las interrupciones enmascarables
    UCA0IFG &= ~UCRXIFG;
    _enable_interrupt();
}

void LED_TurnOn(){
    P1DIR |= BIT0;
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;
	MSP430_Clk_Config();
	eUSCIA1__UART_Init();
	P4_PushButton_Init();
	LED_TurnOn();

    while(1){

    }
	
	return 0;
}


#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void){
    UCA0IFG = 0;
    P1OUT ^= BIT0;  //limpia bandera de interrupcion pendiente(pag. 813) slau367.pdf;
    data[i] = UCA0RXBUF;
    i++;
}


#pragma vector = PORT4_VECTOR
__interrupt void PORT4_ISR(void){
    P4IFG = 0; //limpia bandera de interrupcion
    P1OUT ^= BIT0;
}
