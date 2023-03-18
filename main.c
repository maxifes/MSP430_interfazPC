#include <msp430.h> 
#include <stdint.h>


//Git branch for development

//Configura UART
//- 8 bits de datos.
//- Sin bit de paridad.
//- 1 bit de stop.
//- 9600 baudrate.

#define SIZE_Data 100

int dato = 0;
uint8_t data[100];
unsigned int i = 0;
//uint8_t eUSCIA0_UART_availableData = 0;

void eUSCIA0_UART_send(int data_Tx){
    while((UCA0STATW & UCBUSY) == UCBUSY){}
    UCA0TXBUF = data_Tx; //Dato a enviar (pag.791) manual slau367p.pdf
}

/*
int eUSCIA1_UART_receiveACK_eerase(){
    while(eUSCIA0_UART_availableData == 0){}
        eUSCIA0_UART_data = UCA0RXBUF & 0xFF; //Byte recibido en el registro
                                              //de desplazamiento (pag.791) manual slau367p.pdf
    eUSCIA0_UART_availableData = 0;
    return eUSCIA0_UART_data;
}*/

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;

    CSCTL0_H = CSKEY >> 8;
    CSCTL1 = DCOFSEL_0 | DCORSEL;
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;
    CSCTL0_H = 0;

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

    //Configuracion del LED
    P1DIR |= BIT0;

    P4DIR &= ~BIT5; //habilita pin P4.2 como entrada digital
    P4REN |= BIT5; //Habilita resistencia de pull up o pulldown
    P4OUT |= BIT5; //Lo configura como pull up  //Logica positiva
    P4IES &= ~BIT5; //la bandera de interrupcion se activa con flanco positivo.
    P4IE |= BIT5;   //Activa la interrupciï¿½n en P4.2
    P4IFG = 0;

    _enable_interrupt();

    char message[] = "Hello world \n";
    int position;
    int j;

    while(1){
        for (position = 0; position < sizeof(message); position++) {
            //eUSCIA0_UART_send(message[position]);
        }
        eUSCIA0_UART_send(0x04);
        for (j = 0; j < 30000; j++) {}
        for (j = 0; j < 30000; j++) {}
        //P1DIR ^= BIT0;
    }

	
	return 0;
}


#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void){
    UCA0IFG = 0;
    P1OUT ^= BIT0;                            //limpia bandera de interrupcion pendiente(pag. 813) slau367.pdf;
    //dato = UCA0RXBUF;
    data[i] = UCA0RXBUF;
    i++;
}


#pragma vector = PORT4_VECTOR
__interrupt void PORT4_ISR(void){
    P4IFG = 0; //limpia bandera de interrupcion
    //P1OUT ^= BIT0;
}
