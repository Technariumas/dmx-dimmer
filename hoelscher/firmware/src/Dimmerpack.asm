/**** A P P L I C A T I O N   N O T E   ************************************
*
* Title			: 8ch DMX Dimmerpack (50Hz / 60Hz, single phase)
* Version		: v3.21
* Last updated          : 30.11.2006
* Target		: Transceiver Rev.3.01 [ATmega8515]
*
* written by hendrik hoelscher, www.hoelscher-hi.de
***************************************************************************
 This program is free software; you can redistribute it and/or 
 modify it under the terms of the GNU General Public License 
 as published by the Free Software Foundation; either version2 of 
 the License, or (at your option) any later version. 

 This program is distributed in the hope that it will be useful, 
 but WITHOUT ANY WARRANTY; without even the implied warranty of 
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 General Public License for more details. 

 If you have no copy of the GNU General Public License, write to the 
 Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 

 For other license models, please contact the author.

;***************************************************************************/

.include "m8515def.inc"


/* ***** pin definitions */

#define LED1		PD7
#define LED2		PE0
#define	BIT9		PE2
#define OPTION		PE1
#define USA_MODE	PD4

/* ***** constants */

#define	TABLE_A		0x200
#define TABLE_B		0x400

.equ DMX_FIELD=     0x60 			/* SRAM-Beginn */
.equ DMX_CHANNELS=  8
.equ F_OSC=         8000

#define DMX_EXT

.equ BlinkPos=		0x70
.equ ShtDwnTemp=	0x71			/* Temperature to shut down */
.equ SwitchCh=		0x72			/* Mask of switched channels */


/* ***** special Flags */

#define VALID_DMX 		1 
#define VALID_ZC		2
#define SIGNAL_COMING 	3 
	
#define HOT				6
#define DATA_REFRESHED 	7 


/* ***** global register variables */

#define	tempL		R24			
#define	tempH		R25						
#define	Flags 		R16
#define DMXstate	R17			

#define	null		R2
#define	SREGbuf		R3
#define blink 		R4
#define LEDdelay 	R5				/* prescaler */

#define	status		R6				/* status bits for portc */
#define	SwitchMask  R7

#define	currentL	R18
#define	currentH	R19

#define	dimm_count	R20				/* counter */

#define	timer_startH	R8
#define	timer_startL	R9

/* ***** set time tables for firing angles */ 

.org TABLE_A	/*  50Hz */
	.dw	0x2710, 0x2472, 0x2346, 0x228F, 0x2204, 0x2190, 0x212C, 0x20D2, 0x2081, 0x2037, 0x1FF2, 0x1FB1, 0x1F74, 0x1F3A, 0x1F03, 0x1ECE 
	.dw	0x1E9B, 0x1E6B, 0x1E3C, 0x1E0E, 0x1DE2, 0x1DB8, 0x1D8E, 0x1D66, 0x1D3E, 0x1D18, 0x1CF2, 0x1CCE, 0x1CAA, 0x1C86, 0x1C64, 0x1C42 
	.dw	0x1C21, 0x1C00, 0x1BE0, 0x1BC0, 0x1BA1, 0x1B82, 0x1B64, 0x1B46, 0x1B28, 0x1B0B, 0x1AEE, 0x1AD2, 0x1AB6, 0x1A9A, 0x1A7E, 0x1A63 
	.dw	0x1A48, 0x1A2E, 0x1A13, 0x19F9, 0x19DF, 0x19C6, 0x19AC, 0x1993, 0x197A, 0x1961, 0x1949, 0x1930, 0x1918, 0x1900, 0x18E8, 0x18D1 
	.dw	0x18B9, 0x18A2, 0x188A, 0x1873, 0x185C, 0x1846, 0x182F, 0x1818, 0x1802, 0x17EB, 0x17D5, 0x17BF, 0x17A9, 0x1793, 0x177E, 0x1768 
	.dw	0x1752, 0x173D, 0x1727, 0x1712, 0x16FD, 0x16E8, 0x16D2, 0x16BD, 0x16A9, 0x1694, 0x167F, 0x166A, 0x1655, 0x1641, 0x162C, 0x1618 
	.dw	0x1603, 0x15EF, 0x15DB, 0x15C6, 0x15B2, 0x159E, 0x158A, 0x1576, 0x1562, 0x154E, 0x153A, 0x1526, 0x1512, 0x14FE, 0x14EA, 0x14D6 
	.dw	0x14C2, 0x14AE, 0x149B, 0x1487, 0x1473, 0x1460, 0x144C, 0x1438, 0x1424, 0x1411, 0x13FD, 0x13EA, 0x13D6, 0x13C2, 0x13AF, 0x139B 
	.dw	0x1388, 0x1374, 0x1360, 0x134D, 0x1339, 0x1325, 0x1312, 0x12FE, 0x12EB, 0x12D7, 0x12C3, 0x12AF, 0x129C, 0x1288, 0x1274, 0x1261 
	.dw	0x124D, 0x1239, 0x1225, 0x1211, 0x11FD, 0x11E9, 0x11D5, 0x11C1, 0x11AD, 0x1199, 0x1185, 0x1171, 0x115D, 0x1149, 0x1134, 0x1120 
	.dw	0x110C, 0x10F7, 0x10E3, 0x10CE, 0x10BA, 0x10A5, 0x1090, 0x107B, 0x1066, 0x1052, 0x103D, 0x1027, 0x1012, 0x0FFD, 0x0FE8, 0x0FD2 
	.dw	0x0FBD, 0x0FA7, 0x0F91, 0x0F7C, 0x0F66, 0x0F50, 0x0F3A, 0x0F24, 0x0F0D, 0x0EF7, 0x0EE0, 0x0EC9, 0x0EB3, 0x0E9C, 0x0E85, 0x0E6D 
	.dw	0x0E56, 0x0E3E, 0x0E27, 0x0E0F, 0x0DF7, 0x0DDF, 0x0DC6, 0x0DAE, 0x0D95, 0x0D7C, 0x0D63, 0x0D49, 0x0D30, 0x0D16, 0x0CFC, 0x0CE1 
	.dw	0x0CC7, 0x0CAC, 0x0C91, 0x0C75, 0x0C59, 0x0C3D, 0x0C21, 0x0C04, 0x0BE7, 0x0BC9, 0x0BAB, 0x0B8D, 0x0B6E, 0x0B4F, 0x0B2F, 0x0B0F 
	.dw	0x0AEE, 0x0ACD, 0x0AAB, 0x0A89, 0x0A65, 0x0A41, 0x0A1D, 0x09F7, 0x09D1, 0x09A9, 0x0981, 0x0957, 0x092D, 0x0901, 0x08D3, 0x08A4 
	.dw	0x0874, 0x0841, 0x080C, 0x07D5, 0x079B, 0x075E, 0x071D, 0x06D8, 0x068E, 0x063D, 0x05E3, 0x057F, 0x050B, 0x0480, 0x03C9, 0x029D 



.org TABLE_B	/*  60Hz */
	.dw 8333, 7402, 7224, 7086, 6970, 6870, 6781, 6701, 6627, 6559, 6495, 6436, 6379, 6325, 6274, 6225 
	.dw 6178, 6133, 6090, 6048, 6007, 5968, 5930, 5892, 5856, 5821, 5786, 5753, 5720, 5687, 5656, 5625 
	.dw 5595, 5565, 5536, 5507, 5478, 5451, 5423, 5396, 5370, 5343, 5317, 5292, 5267, 5242, 5217, 5193 
	.dw 5169, 5145, 5122, 5098, 5076, 5053, 5030, 5008, 4986, 4964, 4942, 4921, 4899, 4878, 4857, 4836 
	.dw 4816, 4795, 4775, 4755, 4735, 4715, 4695, 4676, 4656, 4637, 4617, 4598, 4579, 4560, 4541, 4523 
	.dw 4504, 4486, 4467, 4449, 4431, 4412, 4394, 4376, 4358, 4341, 4323, 4305, 4288, 4270, 4253, 4235 
	.dw 4218, 4201, 4183, 4166, 4149, 4132, 4115, 4098, 4081, 4065, 4048, 4031, 4014, 3998, 3981, 3964 
	.dw 3948, 3931, 3915, 3899, 3882, 3866, 3849, 3833, 3817, 3801, 3784, 3768, 3752, 3736, 3720, 3704 
	.dw 3688, 3672, 3655, 3639, 3623, 3607, 3591, 3575, 3559, 3543, 3527, 3511, 3496, 3480, 3464, 3448 
	.dw 3432, 3416, 3400, 3384, 3368, 3352, 3336, 3320, 3304, 3288, 3272, 3256, 3240, 3224, 3208, 3192 
	.dw 3176, 3159, 3143, 3127, 3111, 3095, 3078, 3062, 3046, 3029, 3013, 2997, 2980, 2964, 2947, 2930 
	.dw 2914, 2897, 2880, 2864, 2847, 2830, 2813, 2796, 2779, 2762, 2745, 2728, 2710, 2693, 2675, 2658 
	.dw 2640, 2623, 2605, 2587, 2569, 2551, 2533, 2515, 2496, 2478, 2459, 2441, 2422, 2403, 2384, 2365 
	.dw 2345, 2326, 2306, 2286, 2266, 2246, 2226, 2205, 2185, 2164, 2142, 2121, 2099, 2078, 2056, 2033 
	.dw 2010, 1987, 1964, 1941, 1917, 1892, 1867, 1842, 1816, 1790, 1764, 1736, 1708, 1680, 1650, 1620 
	.dw 1589, 1558, 1524, 1490, 1455, 1418, 1379, 1338, 1294, 1247, 1196, 1140, 1077, 1002, 905,  670

/* ****set interrupt-routines */

.org 0
	
	rjmp reset 				/* reset vector address */
	reti					/* External Interrupt0 Vector Address */
	rjmp zero_crossing		/* External Interrupt1 Vector Address */
	reti					/* Input Capture1 Interrupt Vector Address */
	rjmp compare			/* Output Compare1A Interrupt Vector Address */
	reti					/* Output Compare1B Interrupt Vector Address */
	rjmp zero_crossing_overflow	/* Overflow1 Interrupt Vector Address */
	rjmp LED_indicator		/* Overflow0 Interrupt Vector Address */
	reti					/* SPI Interrupt Vector Address */
	rjmp get_byte			/* UART Receive Complete Interrupt Vector Address */
	reti					/* UART Data Register Empty Interrupt Vector Address */
	reti					/* UART Transmit Complete Interrupt Vector Address */
	reti 					/* Analog Comparator Interrupt Vector Address */
	reti					/* External Interrupt2 Vector Address */
	reti					/* Output Compare0 Interrupt Vector Address */
	reti					/*  EEPROM Interrupt Vector Address */
	reti					/*  SPM complete Interrupt Vector Address */
	reti					/*  SPM complete Interrupt Vector Address */	

reset:

cli

/* ***** set stackpointer */
	ldi	tempL,low(RAMEND)
	ldi tempH,high(RAMEND)
	out	SPL,tempL
	out SPH,tempH

/* ***** WATCHDOG */
	wdr
	ldi tempL, (1<<WDE)|(1<<WDCE)
	out WDTCR, tempL
	ldi tempL, (1<<WDE)|(1<<WDCE)|(1<<WDP2)|(1<<WDP1)
	out WDTCR, tempL
	wdr

/* ***** set Ports */
/*  PortE */
	ldi tempL, 0b00000001
	out DDRE, tempL
	ser	tempL
	out PortE, tempL 					/* LED2 off, BIT9 & OPTION Pullup */

/*  PortA */
	ser tempL
	out DDRA, tempL
	out PortA, tempL 					/* high Outputs */

/*  PortB */
	ldi tempL, 0b00000000
	out	DDRB,  tempL
	ldi tempL, 0b00000011	
	out	PortB, tempL

/*  PortC */
	clr tempL
	out DDRC, tempL
	ser tempL
	out PortC, tempL  					/* Inputs with PullUp for DipSwitch */

/*  PortD */
	ldi tempL, 0b10000100
	out DDRD, tempL
	ldi tempL, 0b01111000
	out PortD, tempL 					/* DMX & Spare , LED1 off */

/* ***** get phase-time for phase zero-crossing control */
		sbis	PinD, USA_MODE
		rjmp	init_phs60

		ldi		tempH,0xD8				//50Hz
		ldi		tempL,0xF2
		rjmp 	init_phs1
  	   init_phs60:
  		ldi		tempH,0xDF				//60Hz
		ldi		tempL,0x75
   	   init_phs1:
		mov		timer_startH,tempH		/* store value */
		mov		timer_startL,tempL

/* ***** initial timer 0 */
		ldi 	tempL,0b00000101		/*  set timer0 to ck/1024 */
		out 	TCCR0,tempL			
				
/* ***** initial timer 1 */
		clr		tempL
		out		TCCR1A, tempL
		ldi 	tempL,0b00000010		/* set timer1 to clk/8 */
		out 	TCCR1B,tempL		 
				
/* ***** initial timer interrupts */
 		ldi 	tempL,0b00000000		/* enable timer0 overflow interrupt */
		sbr		tempL, (1<<TOIE0)	 
		out 	TIMSK,tempL			 
				
/* ***** set interrupt 1 for phase zero-crossing control */
		in 		tempL,MCUCR				/* interrupt by falling edge */ 
		sbr		tempL,(1<<ISC11)		 
		cbr		tempL,(1<<ISC10)		 
		out 	MCUCR,tempL			 

		in 		tempL,GIMSK				/* enable interrupt1 */
		sbr		tempL,(1<<INT1)				 
		out 	GIMSK,tempL 		 
	
		in 		tempL,GIFR				/* initial GIFR */
		sbr		tempL,(1<<INTF0)		/* clear interrupt0 flag */
		sbr		tempL,(1<<INTF1)		/* clear interrupt1 flag */
		out 	GIFR,tempL			 

/* ***** Analog Comparator */
		ldi		tempL, 0b01010010 		/* (bandgap on, ac on, irq on falling edge, irq flag cleared, irg disabled) */
		out 	ACSR,  tempL
		
/* ***** initial var */

		clr 	Flags
		clr 	Flags
		clr 	null

		ldi		tempL, 8
		sts		BlinkPos, tempL
		ldi		tempL, 25
		mov		LEDdelay, tempL

		SWITCHread:
		sbic	EECR, EEWE
		rjmp	SWITCHread
		ldi		tempL, 1
		out		EEARH, null
		out		EEARL, tempL
		sbi		EECR, EERE
		in		SwitchMask, EEDR		/* load switch mask from EE */

		sbic	PinE, OPTION			/* should store new switch pattern? */
		rjmp	initv1
		in		tempH, PinC
		cp		SwitchMask, tempH
		breq	initv_wait
		mov		SwitchMask, tempH

		SWITCHwrite:
		sbic	EECR, EEWE
		rjmp	SWITCHwrite
		out		EEARH, null
		ldi		tempL, 1
		out		EEARL, tempL
		out		EEDR, SwitchMask
		sbi		EECR, EEMWE
		sbi		EECR, EEWE

		initv_wait:
		wdr
		sbis	PinE, OPTION			/* wait till switch cleared */
		rjmp	initv_wait

	   initv1:
	    sts		SwitchCh, SwitchMask
		rcall	init_dmx
		
		cbi     PortD, LED1

/* ***** start working... */

		sei
		wdr								/* enable global interrupt */

forever:

	rjmp	forever	
		
/* ***************************************************************************
 *
 * external interrupt caused by phase-zero-crossing (idle:4.88us, BREAK:6.38us, 
 *
 *************************************************************************** */
zero_crossing:

		in		SREGbuf, SREG			/* save global status */
		push 	tempL
		push	tempH
			
		out 	TCNT1H,timer_startH		/* clear angle-timer */
		out 	TCNT1L,timer_startL		

		sbr 	Flags, (1<<VALID_ZC)	/* message for indicator */

		in		tempL,TIFR				 
		sbr		tempL,(1<<OCF1A)		/* clear compare flag */
		sbr		tempL,(1<<TOV1)			/* clear overflow-flag  */
		out	    TIFR,tempL				

  		in		tempL,TIMSK				
		sbr		tempL,(1<<OCIE1A)		/* enable timer1 compare interrupt */
		sbr		tempL,(1<<TOIE1)		/* enable timer1 overflow interrupt */
		out	    TIMSK,tempL				

		in 		tempL,GIMSK				/* disable ext-interrupt */
		cbr		tempL,(1<<INT1)		
		out 	GIMSK,tempL 			

   zero_common:	
		ser		dimm_count				/* clear counter */

		sbis	PinD, USA_MODE
		rjmp	z_common60
		ldi		ZH,((high(TABLE_A) *2)+0x01)	/* set first compare value */
		rjmp	Z_common1
	   z_common60:
	    ldi		ZH,((high(TABLE_B) *2)+0x01)	/* set first compare value */
 	   Z_common1:
		ldi		ZL,0xfe					
		lpm		tempL, Z+						/* low byte */				
		lpm		tempH, Z						/* high byte */
		add		tempL,timer_startL		/* add start-offset */
		adc		tempH,timer_startH		/* add start-offset */
		out		OCR1AH,tempH			
		out 	OCR1AL,tempL			

		pop		tempH
		pop 	tempL					/* restore global status */
		out 	SREG, SREGbuf
		reti							/* return */	

/* ***************************************************************************
 *
 * T1-overflow interrupt caused by phase-zero-crossing
 *
 *************************************************************************** */
zero_crossing_overflow:	

		in		SREGbuf, SREG			/*  save global status */
		push 	tempL
		push	tempH

		out 	TCNT1H,timer_startH		/* clear angle-timer */
		out 	TCNT1L,timer_startL		

  		in		tempL,TIMSK				 
		sbr		tempL,(1<<OCIE1A)		/* enable timer1 compare interrupt */
		cbr		tempL,(1<<TOIE1)		/* disable timer1 overflow interrupt */
		out		TIMSK,tempL				

		in 		tempL,GIFR				/* clear interrupt0 flag */
		sbr		tempL,(1<<INTF1)		 
		out 	GIFR,tempL				

		rjmp	zero_common

/* ***************************************************************************
 *
 * check next firing angle
 *
 *************************************************************************** */
compare:
		in		SREGbuf, SREG			/*  save global status */
		push 	tempL
		push	tempH

		ldi		tempL,DMX_CHANNELS
		clr		status
		ldi		ZH,high(DMX_FIELD)		/* set data-pointer */	
		ldi		ZL,low(DMX_FIELD)			

  test_stat:	
  		ld		tempH,Z+				/* get data  */
		cp		tempH, dimm_count
		ror		status
		dec		tempL					/* goto next channel */
		breq	finish					 
		rjmp	test_stat

  finish:
  		sbrs	Flags, HOT				/* if too hot -> shut down! */
		rjmp	cf_1
		clr		status 
		com		status
      cf_1:
		out 	PORTA,status
		
  get_value:
		dec		dimm_count
		breq	end_time

		sbis	PinD, USA_MODE
		rjmp	gv_60
		ldi		ZH,(high(TABLE_A) *2)	/* compare value */
		rjmp	gv_1
	   gv_60:
	    ldi		ZH,(high(TABLE_B) *2)	/* compare value 60Hz */
 	   gv_1:
		mov		ZL, dimm_count
		lsl		ZL			 
		adc		ZH,null					  
	
		lpm		tempL, Z+				/* load next timer1-compare-value */				 
		lpm		tempH, Z						 				 

		add		tempL,timer_startL		/* add start-offset */
		adc		tempH,timer_startH		 

		sbiw	tempL,0x03				/* test if next timer1-compare-value  (next angle -3) */
		in		currentL,TCNT1L			/* isn't already gone */
		in		currentH,TCNT1H
		cp		tempL,currentL
		cpc		tempH,currentH
		brge	compare_ok
		rjmp	get_value				/* else get next value */

  compare_ok:
  		adiw	tempL,0x03				/*  (next angle +3)  */
		out 	OCR1AH,tempH			/* set next timer1-compare-value */
		out		OCR1AL,tempL			 
		
		rjmp	exit_compare

  end_time:
  		in		tempL,TIMSK				/* disable timer1 compare interrupt */
		cbr		tempL,(1<<OCIE1A)		 
		out		TIMSK,tempL				 

		ldi		tempL,0xff				/* turn portA off (safe) */
		out		PORTA,tempL				

		in		tempL,TIMSK				/* check for timer-overflow-interrupt */
		sbrc	tempL,TOIE1				/* if timer1 overflow interrupt is enabled */
		rjmp	exit_compare			/*   return */

		in 		tempL,GIFR				/* clear interrupt1 flag */
		sbr		tempL,(1<<INTF1)		
		out 	GIFR,tempL				

		in 		tempL,GIMSK				/* enable external interrupt1 */
		sbr		tempL,(1<<INT1)			
		out 	GIMSK,tempL 			/* wait for next zero-crossing */

	exit_compare:
		pop		tempH
		pop		tempL
		out		SREG, SREGbuf
		reti							



/* ***************************************************************************
 *
 * manipulation of dmx data
 *
 *************************************************************************** */
 clc_dmx:
 		lsr		SwitchMask				/* should this ch be switched? */
		brcs	clc_dmx_finish			/* no */
		cpi		tempH, 127
		brsh	clc_dmx_hi
		clr		tempH					/* ch off */
		rjmp	clc_dmx_finish
	  clc_dmx_hi:
		ser		tempH					/* ch on */
	  clc_dmx_finish:
	    cpi		XL, low(DMX_FIELD +DMX_CHANNELS -1)
		brne	clc_dmx_exit
		lds		SwitchMask, SwitchCh	/* reload switch mask */
	  clc_dmx_exit:
		ret		
			
 			

/* ***************************************************************************
 *
 * LED INDICATOR 
 *
 *************************************************************************** */
LED_indicator:
		wdr								/* reset Watchdog */
		in		SREGbuf, SREG
		push	tempL

		dec	  	LEDdelay				/* clk/(256*1024*2) => 20Hz */			
		brne  	no_ind
		ldi   	tempL, 2
		mov   	LEDdelay, tempL		

working_LED:
		sbrc	Flags, DATA_REFRESHED	/* should flash? */
		rjmp	data_flash	
		sbi		PortE, LED2				/* LED off */	


Error_LED:
		lsr   	blink
		sbrc  	blink, 0				/* wenn 1st bit HI */
		rjmp  	on
		sbi   	PortD, LED1
		rjmp  	ind_tst
	 on:
	 	cbi  	PortD, LED1

     ind_tst:
	    lds		tempL, BlinkPos
		dec	  	tempL					/* ist blink durchrotiert? */
		breq	change_pat
		sts		BlinkPos, tempL
		rjmp	no_ind
	 		 	
/* wenn durchrotiert (blink = 0) */
	 change_pat:
	    ldi		tempL, 8
		sts		BlinkPos, tempL	
		clr		tempL		
		sbrs 	Flags, VALID_DMX
		ldi	 	tempL, 0b00001010
		sbrs 	Flags, SIGNAL_COMING
		ldi	 	tempL, 0b00000010
		sbrs 	Flags, VALID_ZC
		ldi  	tempL, 0b10101010

/* *** Temperature Monitoring */
	  	sbic	ACSR, ACO
		rjmp	too_hot
		cbr		Flags, (1<<HOT)			/* temp is OK */
		rjmp	tm1
	too_hot:							/* temp is too high */
		ser		tempL
		sbr		Flags, (1<<HOT)
		
   tm1: mov  	blink, tempL

		cbr		Flags, (1<<VALID_DMX)|(1<<VALID_ZC)|(1<<SIGNAL_COMING)
	no_ind:
		pop		tempL
		out		SREG, SREGbuf
		reti

    data_flash:
		cbr		Flags, (1<<DATA_REFRESHED)
	    sbis 	PortE,LED2				/* blinken green */
		rjmp 	off
		cbi		PortE,LED2
		rjmp 	Error_LED
		off:
		sbi 	PortE,LED2
		rjmp 	Error_LED



.include "lib_dmx_in.asm"

nix: rjmp nix



	
	


