#include <msp430.h> 
#include <stdint.h>


//Git branch for development
/*
 * Recibe un archivo .hex completo,
 * Los datos de interes de cada linea
 * se guardan en los vectores:
 * frame_1 (Primera linea de datos del archivo hex)
 * frame_2 (Segunda linea de datos del archivo hex)
 * y asi consecutivamente para las 5 lineas del archivo hex de prueba
 * */

//Configura UART
//- 8 bits de datos.
//- Sin bit de paridad.
//- 1 bit de stop.
//- 9600 baudrate.

/*
 * Trama recibida desde el dispositivo externo
 * data[0] - Numero de bytes de datos
 * dato[1] - Address MSB (Sin offset)
 * dato[2] - Address LSB (Sin offset)
 * dato[3] - Primer dato
 * dato[3+data[0]] - dato n (dato final)
 * dato[4+data[0]] - dato n (checksum)
 * */

#define SIZE_BUFFER 100
#define START_BYTE 0xF
#define END_BYTE 0xF0
#define ACK_BYTE 0x79
#define NACK_BYTE 0x7F


uint8_t update_Code_Buffer[SIZE_BUFFER];
uint8_t update_Code_Byte_Count = 0;
uint8_t update_Code_first_Byte = 1;
uint8_t update_Code_enable = 0;
uint8_t update_Code_frame_ready = 0;

uint8_t frame_1[25] = {0};
uint8_t frame_2[25] = {0};
uint8_t frame_3[25] = {0};
uint8_t frame_4[25] = {0};
uint8_t frame_5[25] = {0};

void eUSCIA0_UART_send(int data_Tx){
    while((UCA0STATW & UCBUSY) == UCBUSY){}
    UCA0TXBUF = data_Tx; //Dato a enviar (pag.791) manual slau367p.pdf
}

void MSP430_Clk_Config(){
    CSCTL0_H = CSKEY >> 8;
    CSCTL1 = DCOFSEL_0 | DCORSEL;
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;
    CSCTL0_H = 0;
}

void eUSCIA0__UART_Init(){
    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 |= UCSSEL__SMCLK;
    UCA0BRW |= 6;
    UCA0MCTLW |= 0x20<<8 | UCOS16 | 8<<4;

    P2SEL0 &= ~(BIT0 | BIT1);                   //P2SEL0.x = 0
    P2SEL1 |= BIT0 | BIT1;                      //P2SEL1.x = 1; Selecciona la función de UART en P2.1 y P2.0
    PM5CTL0 &= ~LOCKLPM5;

    UCA0CTLW0 &= ~UCSWRST;

    UCA0IE |= UCRXIE;                           //Habilita interrupción de recepción
                      //Habilita la las interrupciones enmascarables
    UCA0IFG &= ~UCRXIFG;
    _enable_interrupt();
}

void LED_Init(){
    P1DIR |= BIT0;
}

void LED_TurnOn(){
    P1OUT |= BIT0;
}

void LED_TurnOff(){
    P1OUT &= ~BIT0;
}

void LED_Toggle(){
    P1OUT ^= BIT0;
}

uint8_t Frame_Verify_Checksum(uint8_t data[]){
    uint8_t i = 0;
    uint8_t local_checksum = 0;
    uint8_t received_checksum = 0;
    for(i = 0; i<= data[0]+2; i++){
        local_checksum ^=  data[i];
    }
    received_checksum = data[data[0]+3];
    if(local_checksum == received_checksum){
        return 1;
    }else{
        return 0;
    }
}

void Split_Vector(uint8_t ready_frames_count){
    uint8_t i = 0;
    switch (ready_frames_count) {
        case 1:
            for(i = 0;i <= 25; i++){
                frame_1[i] = update_Code_Buffer[i];
            }
            break;
        case 2:
            for(i = 0;i <= 25; i++){
                frame_2[i] = update_Code_Buffer[i];
            }
            break;
        case 3:
            for(i = 0;i <= 25; i++){
                frame_3[i] = update_Code_Buffer[i];
            }
            break;
        case 4:
            for(i = 0;i <= 25; i++){
                frame_4[i] = update_Code_Buffer[i];
            }
            break;
        case 5:
            for(i = 0;i <= 25; i++){
                frame_5[i] = update_Code_Buffer[i];
            }
            break;
        default:
            break;
    }
    for(i = 0;i <= 25; i++){
        update_Code_Buffer[i] = 0;
    }
}

int main(void)
{
    uint8_t ready_frames_count = 0;

	WDTCTL = WDTPW | WDTHOLD;
	MSP430_Clk_Config();
	eUSCIA0__UART_Init();
	LED_Init();
	LED_TurnOn();

    while(1){
        if(update_Code_enable == 1){
            if(update_Code_frame_ready == 1){
                if(Frame_Verify_Checksum(update_Code_Buffer) == 1){
                    ready_frames_count++;
                    Split_Vector(ready_frames_count);

                    /*
                     * Aca va la funcion para la escritura en la FRAM
                     * **********************************************
                     * **********************************************
                     * **********************************************
                     * El dispositivo externo envía los datos con el siguiente orden
                     * update_Code_Buffer[0] - Numero de bytes de datos
                     * update_Code_Buffer[1] - Address MSB (Sin offset)
                     * update_Code_Buffer[2] - Address LSB (Sin offset)
                     * update_Code_Buffer[3] - Primer dato
                     * update_Code_Buffer[3+data[0]] - dato n (dato final)
                     * update_Code_Buffer[4+data[0]] - dato n (checksum)
                     *
                     * */
                    eUSCIA0_UART_send(ACK_BYTE);
                }else{
                    eUSCIA0_UART_send(NACK_BYTE);
                }
                LED_Toggle();
                update_Code_frame_ready = 0;
            }
        }else{
            ready_frames_count = 0;
        }
    }
	
	return 0;
}


#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void){
    UCA0IFG = 0;
    update_Code_frame_ready = 0;
    int start_byte_received = 0;

    if(update_Code_first_Byte == 1){ //¿Es una nueva trama?
        start_byte_received = UCA0RXBUF;
        if(start_byte_received == START_BYTE){
           update_Code_first_Byte = 0;
           P1OUT ^= BIT0;
           UCA0TXBUF = ACK_BYTE; //Dato a enviar (pag.791) manual slau367p.pdf
           update_Code_enable = 1; //Habilita programacion
        }else if(start_byte_received == END_BYTE){
           update_Code_first_Byte = 1;
           UCA0TXBUF = ACK_BYTE;
           update_Code_enable = 0; //Deshabilita reprogramación
        }else{//Si el primer byte no es el byte de inicio o el byte de fin, entonces es un dato.
            update_Code_first_Byte = 0;
            update_Code_Buffer[update_Code_Byte_Count] = start_byte_received;
            update_Code_Byte_Count++;
        }
    }else{
        update_Code_Buffer[update_Code_Byte_Count] = UCA0RXBUF;
        update_Code_Byte_Count++;
        if(update_Code_Byte_Count == 4 + update_Code_Buffer[0]){
            update_Code_Byte_Count = 0;
            update_Code_frame_ready = 1;
            update_Code_first_Byte = 1;
        }
    }
}
