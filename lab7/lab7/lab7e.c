/* Lab 7 Part E
   Name: Pengzhao Zhu
   Section#: 112D
   TA Name: Chris Crary
   Description: This program add to the Part C of lab 7.
				This program configures ADCA as the source for DMA. It also configures the DMA to transfer recorded ADC data
				to the USARTD0 system everytime ADCA conversion is complete. Connect to the data visualizer in Atmel
*/



#include <avr/io.h>
#include <avr/interrupt.h>


void CLK_32MHZ(void);
void ADC(void);
void TIMER_INIT(void);
void USARTD0_init(void);
void DMA_INIT(void);

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
	DMA_INIT();
	
	while(1) {

	}
	
	return 0;
}

void DMA_INIT(void) {
	DMA_CTRL=DMA_ENABLE_bm | DMA_DBUFMODE_DISABLED_gc;    //enable DMA and disable duffer buffer mode
	
	DMA_CH0_REPCNT=0x00;    //repeat count of 0, which is unlimited repeat
	DMA_CH0_ADDRCTRL=0b10001000; //source address and destination reloaded with initial value at end of each burst
	//source and destination does not increment
	DMA_CH0_TRIGSRC=  DMA_CH_TRIGSRC_ADCA_CH0_gc;    //trigger source for DMA as event channel 1
	

	DMA_CH0_SRCADDR0= (uint8_t)&ADCA_CH0_RES;                  //source address is ADCA_CH0RES
	DMA_CH0_SRCADDR1= ((uint16_t)&ADCA_CH0_RES) >> 8;
	DMA_CH0_SRCADDR2= ((uint32_t)&ADCA_CH0_RES) >> 16;
	
	DMA_CH0_DESTADDR0=(uint8_t)&USARTD0_DATA;                 //destination address is USARTD0_DATA
	DMA_CH0_DESTADDR1=((uint16_t)&USARTD0_DATA) >> 8;
	DMA_CH0_DESTADDR2=((uint32_t)&USARTD0_DATA) >> 16;
	
	DMA_CH0_CTRLA=DMA_CH_ENABLE_bm | DMA_CH_REPEAT_bm | DMA_CH_SINGLE_bm | DMA_CH_BURSTLEN_1BYTE_gc;      
	//repeat mode, single shot data transfer
	//burst mode defaults to 00=1 byte
	
}



void ADC(void) {
	
	ADCA_REFCTRL=ADC_REFSEL_AREFB_gc;      //adc reference as PORTB aref. start scanning on channel 0
	ADCA_PRESCALER=ADC_PRESCALER_DIV64_gc;                //512 prescaler or adc clock
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
	EVSYS_CH1MUX=EVSYS_CHMUX_ADCA_CH0_gc; //set ADCA CH0 conversion complete as source for CH1 event
	
	
}

void USARTD0_init(void)
{
	PORTD_DIRSET=PIN3_bm;   //set transmitter as output
	PORTD_DIRCLR=PIN2_bm;	 //set receiver as input
	
	USARTD0_CTRLB=0x18;  //enable receiver and transmitter
	USARTD0_CTRLC=0b00000011;
	//USARTD0_CTRLC= USART_CHSIZE_8BIT_gc | USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc; //USART asynchronous, 8 data bit, no parity, 1 stop bit
	
	USARTD0_BAUDCTRLA= 0x83;    //load lowest 8 bits of BSEL
	USARTD0_BAUDCTRLB= 0xD0; //load BSCALE and upper 4 bits of BSEL. bitwise OR them
	
	//PORTD_OUTSET= PIN3_bm;   //set transit pin idle
	
	
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
