cpu 8086
bits 16

org 100h

section .text


xor ax, ax
mov dx, init_msg_start
mov cx, init_msg_start_len
mov bx, STD_OUT
mov ah, WRITE_FILE
int 0x21

;set interrupt vector through dos service

SettingIntVector:

cli

;set interrupt vector directly inside interrupt table
;interrupt vectors table is located at addr 0
mov ax, 0
mov es, ax

;target interrupt vector address is 0:(int_num*4)
mov ax, IntHandlerCode
mov [es:(0xb * 4)], ax

;and segment for interrupt handler routine is the next word
mov ax, cs
mov [es: ((0xb * 4) + 2)], ax

;unmask irq
in al, 21h         ; Read existing bits.
and al, 0xf7       ; Unmask IRQ3 (serial port com2)
out 21h, al

sti

;exit
; DX = paragraphs to reserve
mov dx, ((pgm_len + 15) / 16)+10h
xor ax, ax
mov ah, 0x31                         ; function 31h = terminate and stay resident
int 21h



IntHandlerCode:
cli


; using bios int 10h because dos int 21h is unstable for int handler
;new line
mov al, 0x0d
mov ah, 0eh
int 10h
mov al, 0x0a
mov ah, 0eh
int 10h

mov al, 't'
mov ah, 0eh
int 10h

mov al, 'm'
mov ah, 0eh
int 10h

mov al, 'r'
mov ah, 0eh
int 10h

mov al, ' '
mov ah, 0eh
int 10h

mov al, 'd'
mov ah, 0eh
int 10h

mov al, 'o'
mov ah, 0eh
int 10h

mov al, 'n'
mov ah, 0eh
int 10h

mov al, 'e'
mov ah, 0eh
int 10h

;new line
mov al, 0x0d
mov ah, 0eh
int 10h
mov al, 0x0a
mov ah, 0eh
int 10h


;;;;   irq handler action

; wait for board button signal to end
; use system timer (channel 0)
	push si
	xor ax, ax
	xor bx, bx
	xor dx, dx

	mov  si, 54  ;3s delay *18
	mov  ah, 0
	int  1ah

	mov  bx, dx
	add  bx, si

delay_loop:

	int  1ah
	cmp  dx, bx
	jne  delay_loop

	pop  si
				
				
; send 5s delay to timer board
	mov ax, 0x5
; 0x220 is a dip-selected board address
; ...1 at the least significant bit of address triggers "shutdown on count finish" mode
	mov dx, 0x221
	out dx, al

;;;;


;End-of-interrupt (EOI) is signaled to 8259 by writing 20H into ICR
mov AL,20H
out 20H,AL

sti

iret


pgm_len equ $ - SettingIntVector    ; program length


section .data
    STD_OUT equ 1
    WRITE_FILE equ 0x40

    init_msg_start db 'Setting Timer interrupt vector'
    init_msg_start_len equ $ - init_msg_start
