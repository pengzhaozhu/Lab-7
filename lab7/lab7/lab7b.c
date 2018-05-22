/* Lab 7 Part B
   Name: Pengzhao Zhu
   Section#: 112D
   TA Name: Chris Crary
   Description: This program add to the Part A of lab 7.
				This program configures the timer to overflow at a rate of 2 KHZ. It also configures timer overflow 
				as the source for Event Channel 0
*/



#include <avr/io.h>
#include <avr/interrupt.h>


void CLK_32MHZ(void);
void ADC(void);
void TIMER_INIT(void);


uint16_t adc;

double event_timer = ((32000000*(1/20000))/64);

int main(void)
{
	CLK_32MHZ();   //call 32MHZ  clock
	ADC();         //initialize ADC system
	TIMER_INIT();   //initialize timer system
	//8 bit unsigned adc with 2.5 V as reference. v=(1/102)adc
	
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
