#include "LPC17xx.h"

/* -------- CONFIG -------- */
#define SEG_PORT LPC_GPIO2
#define SEG_MASK 0xFF

#define DIG_PORT LPC_GPIO1
#define DIG1 (1<<29)
#define DIG2 (1<<28)

#define LED_PORT LPC_GPIO0
#define LED_MASK ((1<<0)|(1<<1)|(1<<10)|(1<<11))

#define BUZZ_PORT LPC_GPIO1
#define BUZZ (1<<23)

/* -------- GLOBAL -------- */
volatile int test_index = 0;   // which test case

/* COMMON CATHODE */
unsigned char seg_lut[16] = {
    0x3F,0x06,0x5B,0x4F,
    0x66,0x6D,0x7D,0x07,
    0x7F,0x6F,0x77,0x7C,
    0x39,0x5E,0x79,0x71
};

void delay_small() {
    for(int i=0;i<10000;i++);
}

/* -------- EINT1 ? NEXT TEST CASE -------- */
void EINT1_IRQHandler(void) {
    LPC_SC->EXTINT = (1<<1);

    test_index++;
    if(test_index > 4) test_index = 0;
}

/* -------- DISPLAY -------- */
void display_hex(int num) {

    int high = (num >> 4) & 0x0F;
    int low  = num & 0x0F;

    DIG_PORT->FIOCLR = DIG1 | DIG2;

    // RIGHT digit
    SEG_PORT->FIOCLR = SEG_MASK;
    SEG_PORT->FIOSET = seg_lut[low];
    DIG_PORT->FIOSET = DIG1;
    delay_small();
    DIG_PORT->FIOCLR = DIG1;

    // LEFT digit
    SEG_PORT->FIOCLR = SEG_MASK;
    SEG_PORT->FIOSET = seg_lut[high];
    DIG_PORT->FIOSET = DIG2;
    delay_small();
    DIG_PORT->FIOCLR = DIG2;
}

void display_led(int num) {

    LED_PORT->FIOCLR = LED_MASK;

    if(num & 1) LED_PORT->FIOSET = (1<<0);
    if(num & 2) LED_PORT->FIOSET = (1<<1);
    if(num & 4) LED_PORT->FIOSET = (1<<10);
    if(num & 8) LED_PORT->FIOSET = (1<<11);
}

/* -------- MAIN -------- */
int main() {

    SystemInit();

    SEG_PORT->FIODIR |= SEG_MASK;
    DIG_PORT->FIODIR |= DIG1 | DIG2;
    LED_PORT->FIODIR |= LED_MASK;

    BUZZ_PORT->FIODIR |= BUZZ;
    BUZZ_PORT->FIOCLR = BUZZ;

    /* -------- EINT1 SETUP -------- */
    LPC_PINCON->PINSEL4 &= ~(3<<22);
    LPC_PINCON->PINSEL4 |=  (1<<22);

    LPC_SC->EXTMODE |= (1<<1);
    LPC_SC->EXTPOLAR &= ~(1<<1);

    NVIC_EnableIRQ(EINT1_IRQn);

    /* -------- TEST CASES -------- */
    int A[5] = {12, 8, 7, 10, 15};
    int B[5] = { 6, 2, 1,  5,  3};

    int result;

    while(1) {

        int a = A[test_index];
        int b = B[test_index];

        
        for(int mode=0; mode<4; mode++) {

            switch(mode) {
                case 0: result = a + b; break;
                case 1: result = a - b; break;
                case 2: result = a * b; break;
                case 3:
                    if(b != 0) result = a / b;
                    else result = 0;
                    break;
            }

            display_led(result);

            for(int t=0;t<3000;t++) {
                display_hex(result);
            }
        }
    }
}