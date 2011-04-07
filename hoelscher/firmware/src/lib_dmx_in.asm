;**** A P P L I C A T I O N   N O T E   ************************************
;*
;* Title		: DMX512 reception library
;* Version		: v1.2
;* Last updated	        : 04.06.06
;* Target		: Transceiver Rev.3.01 [ATmega8515]
;*
;* written by hendrik hoelscher, www.hoelscher-hi.de
;***************************************************************************
; This program is free software; you can redistribute it and/or 
; modify it under the terms of the GNU General Public License 
; as published by the Free Software Foundation; either version2 of 
; the License, or (at your option) any later version. 
;
; This program is distributed in the hope that it will be useful, 
; but WITHOUT ANY WARRANTY; without even the implied warranty of 
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
; General Public License for more details. 
;
; If you have no copy of the GNU General Public License, write to the 
; Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
;
; For other license models, please contact the author.
;
;***************************************************************************


 /************* Definitions in main source **********************
		
		#define DMX_FIELD	 0x60	//base address of DMX array
		#define DMX_CHANNELS 8		//no. of channels
		#define DMX_CHANGED  0		//DMX flag: indicates ch changes
		#define DMX_BT_CH	 1		//internal flag
		#define DMXstate	 R18	//status reg for DMX-ISR (r16-r31)
	  //X-Pointer reserved for DMX ISR!!

		#define F_OSC		 8000  	//oscillator freq. in kHz (typical 8MHz or 16MHz)

 **************************************************************/

	
init_dmx:
	;**** PortD
	in	tempL, DDRD
	sbr	tempL, (1<<2)
	out	DDRD, tempL
	
	in	tempL, PortD
	cbr	tempL, (1<<2)		;enable reception
	out	PortD, tempL
	
	;**** USART
    	ldi 	tempL, ((F_OSC/4000)-1) ;set Usart to 250 kbaud
    	out 	UBRRL, tempL
	
    	clr 	tempL
    	out 	UBRRH, tempL
	
    	ldi 	tempL, (1<<URSEL)|(3<<UCSZ0)|(1<<USBS)
    	out 	UCSRC, tempL
	
	in	tempL,UDR
 	clr	tempL
	out	UCSRA,tempL
	
	sbi 	UCSRB,RXCIE
	sbi 	UCSRB,RXEN			

#ifndef USE_ADR
	;**** PortC
	clr 	tempL
	out 	DDRC, tempL
	ser 	tempL
	out 	PortC, tempL  		;Inputs with PullUp for DipSwitch
	
	;**** PortE
	in	tempL, DDRE
	cbr	tempL, (1<<1)|(1<<2)
	out 	DDRE, tempL
	in	tempL, PortE
	sbr	tempL, (1<<1)|(1<<2)
	out 	PortE, tempL 		;PE2 & OPTION Pullup
#endif

	;**** memory
	ldi	XH, high(DMX_FIELD)
	ldi	XL,  low(DMX_FIELD)
	ldi	tempL,0x00		;set all channels to 0
	ldi 	DMXstate, DMX_CHANNELS

write_RxD:	
	st 	X+,tempL
	dec	DMXstate
	brne	write_RxD
	
	//clr 	DMXstate		;set DMXstate to 'wait for Reset' (done by counting ;-)
	ret


/* ***********************************************************************************************************
 *					DMX reception ISR
 ************************************************************************************************************/
get_byte:
		in      SREGbuf, SREG
		push	tempH
		push	tempL		

		in		tempL,UCSRA		;get status register before data register
		in		tempH,UDR
#ifdef SIGNAL_COMING
		sbr		Flags, (1<<SIGNAL_COMING)	;indicator flag: USART triggered
#endif	
		sbrc	tempL,DOR		;check for overrun -> wait for reset
		rjmp	overrun	

		sbrc	tempL,FE		;check for frame error -> got reset
		rjmp	frame_error	
		
		cpi		DMXstate, 1   	;check for startbyte (must be 0)
		breq	startbyte

  		cpi 	DMXstate, 2   	;check for start adress
		breq    startadr
  		
 		cpi 	DMXstate, 3	
  		brsh	handle_byte

	gb_finish:
		pop		tempL
		pop		tempH
		out     SREG, SREGbuf	
		reti			

	startbyte:
		tst 	tempH	 				;if null-> Startbyte
		brne 	overrun
		inc  	DMXstate 				;next -> start address

#ifndef USE_ADR
		in 		XL, PinC 				;get start address
		com     XL
		clr		XH
		sbis	PinE, PE2				;9th bit
		inc		XH
#endif
#ifdef USE_ADR
		rcall	get_address
#endif
		rjmp	gb_finish

 	startadr:
		sbiw	XH:XL, 1
		brne	gb_finish
		ldi 	XH, high(DMX_FIELD)
		ldi		XL,  low(DMX_FIELD)
		inc  	DMXstate 				;next-> data handler
		
	handle_byte:
#ifdef DMX_EXT							;external manipulation of DMX data?
		rcall	clc_dmx
#endif			
		ld		tempL, X				;get old val
		cp		tempL, tempH
		breq	gb_noChange
		st		X, tempH
#ifdef DATA_REFRESHED
		sbr		Flags, (1<<DATA_REFRESHED) ;indicator flag: ch changed
#endif
       gb_noChange:
	    adiw	XH:XL, 1

		cpi		XL, low(DMX_FIELD +DMX_CHANNELS)
		brne	gb_finish
		cpi		XH, high(DMX_FIELD +DMX_CHANNELS)
		brne	gb_finish
#ifdef VALID_DMX
		sbr		Flags, (1<<VALID_DMX)	;indicator flag: all ch received
#endif	
		clr		DMXstate
		rjmp	gb_finish
	
 	frame_error:
		ldi 	DMXstate, 1				;FE detected as break
		cbi		UCSRA,FE				;further framing errors, caused by longer breaks
		rjmp	gb_finish

	overrun:
	 	clr 	DMXstate				;wait for frame-error
		cbi		UCSRA,DOR				
		rjmp	gb_finish
