/* Lab 7 Part C
   Name: Pengzhao Zhu
   Section#: 112D
   TA Name: Chris Crary
   Description: This program add to the Part B of lab 7.
				This program configures USARTD0 to have a baud rate of 115200 with one stop bit, one start bit, 
				and no parity bit. 
*/



#include <avr/io.h>
#include <avr/interrupt.h>


void CLK_32MHZ(void);
void ADC(void);
void TIMER_INIT(void);
void USARTD0_init(void);

uint16_t adc;

double BSELHIGH = (((4)*((32000000/(16*115200))-1))>>8);   //bscale of -2
double BSEL= ((4)*((32000000/(16*115200))-1));			//bscale of -2

//double event_timer = ((32000000*(1/20000))/64);   //PER value to trigger event0, which then trigger ADC channel 0 conversion

int main(void)
{
	CLK_32MHZ();   //call 32MHZ  clock
	ADC();         //initialize ADC system
	TIMER_INIT();   //initialize timer system
	//8 bit unsigned adc with 2.5 V as reference. v=(1/102)adc
	
	USARTD0_init();

	
	
	
	while(1) {
		ADCA_CTRLA=ADC_ENABLE_bm | ADC_CH0START_bm;
		while((ADCA_CH0_INTFLAGS & 0x01)!= 0x01);
		adc=ADCA_CH0_RES;
		ADCA_CH0_INTFLAGS=0x01;
	}
	
	return 0;
}


void ADC(void) {
	
	ADCA_REFCTRL=ADC_REFSEL_AREFB_gc;      //adc reference as PORTB aref. start scanning on channel 0
	ADCA_PRESCALER=ADC_PRESCALER_DIV512_gc;                //512 prescaler or adc clock
	ADCA_CTRLB=ADC_RESOLUTION_8BIT_gc ;     //unsigned mode, 8 bit resolution, no free run
	PORTA_DIRCLR= PIN0_bm; //PA0 as input
	ADCA_CH0_CTRL=ADC_CH_INPUTMODE_SINGLEENDED_gc;    //single ended mode
	ADCA_CH0_MUXCTRL=ADC_CH_MUXPOS_PIN0_gc;      //mux control
	
	ADCA_CTRLA=ADC_ENABLE_bm | ADC_CH0START_bm;
	
	ADCA_EVCTRL=ADC_SWEEP_0_gc | ADC_EVSEL_0123_gc | ADC_EVACT_CH0_gc; //only sweep channel 0, 0123 event as selected inputs,
	// then furtuhur reduced down to use EVENT0 to
	// trigger ADC CHANNEL0
	
	
	
}

void TIMER_INIT(void) {
	TCC0_CNT=0x00;   //set CNT to zero
	TCC0_PER=25;    //timer per value to output 1760 Hz sine wave
	TCC0_CTRLA=TC_CLKSEL_DIV64_gc; //
	
	EVSYS_CH0MUX=EVSYS_CHMUX_TCC0_OVF_gc; //set TCC0 OVF as the source for CH0 event
	
	
}

void USARTD0_init(void)
{
	PORTD_DIRSET=PIN3_bm;   //set transmitter as output
	PORTD_DIRCLR=PIN2_bm;	 //set receiver as input
	
	USARTD0_CTRLB=0x18;  //enable receiver and transmitter
	USARTD0_CTRLC= USART_CHSIZE_8BIT_gc | USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc; //USART asynchronous, 8 data bit, no parity, 1 stop bit
	
	USARTD0_BAUDCTRLA= (uint8_t) BSEL;    //load lowest 8 bits of BSEL
	USARTD0_BAUDCTRLB= (((uint8_t) BSELHIGH) | 0xE0); //load BSCALE and upper 4 bits of BSEL. bitwise OR them
	
	PORTD_OUTSET= PIN3_bm;   //set transit pin idle
	
	
}



void CLK_32MHZ(void)
{
	
	OSC_CTRL=0x02;     //select the 32Mhz osciliator
	while ( ((OSC_STATUS) & 0x02) != 0x02 );   //check if 32Mhz oscillator is stable
	//if not stable. keep looping
	
	CPU_CCP= 0xD8;                       //write IOREG to CPU_CCP to enable change
	CLK_CTRL= 0x01;						//select the 32Mhz oscillator
	CPU_CCP= 0xD8;						//write IOREG to CPU_CCP to enable change
	CLK_PSCTRL= 0x00;					//0x00 for the prescaler
	
}


