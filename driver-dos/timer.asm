; Use DOS 2.0's service 40 to output a length-delimited string

cpu 8086
bits 16

org 100h

section .text

start:

	;store registers
	pushf

	push ax
	push bx
	push cx
	push dx

	push si
	push di


	;init stack
	mov ax, cs
	mov ds, ax
	mov ss, ax
	;mov sp, 0x476f               ; max address for 64kb ram
	
	;say hi
	xor ax, ax
	mov dx, start_msg          ; message's address in dx
	mov cx, start_msg_len
	mov bx, STD_OUT           ; Device/handle: standard out (screen)
	mov ah, WRITE_FILE       ; ah=0x40 - "Write File or Device"
	int 0x21                         ; call dos services

	;parse timeout integer and put on stack
	call parse_input_val
	
	mov bx, sp
	mov ax, [bx]
	;outpup timeout on bus
	mov dx, timer_board_address
	out dx, al
	
	;if parsed ok - print passed timeout value
	xor ax, ax
	mov dx, request_msg_start
	mov cx, request_msg_start_len
	mov bx, STD_OUT
	mov ah, WRITE_FILE
	int 0x21
	
	call print_timeout_val
	
	xor ax, ax
	mov dx, request_msg_end
	mov cx, request_msg_end_len
	mov bx, STD_OUT
	mov ah, WRITE_FILE
	int 0x21
	;end print
	
	;exit
	mov ah, 0x4c                  ; "terminate program" sub-function
	int 0x21                          ; call dos services
	
	
	
	;;;
	
	;print bad cmdline args and exit
bad_args:
	mov dx, bad_args_msg_1
	mov cx, bad_args_msg_len_1
	mov bx, STD_OUT
	mov ah, WRITE_FILE
	int 0x21
	
	mov dx, bad_args_msg_2
	mov cx, bad_args_msg_len_2
	mov bx, STD_OUT
	mov ah, WRITE_FILE
	int 0x21
	
	
	;restore registers
	popf

	pop ax
	mov di, ax
	pop ax
	mov si, ax

	pop ax
	mov dx, ax
	pop ax
	mov cx, ax
	pop ax
	mov bx, ax
	pop ax

	;exit
	mov ah, 0x4c
	int 0x21
	

;get timeout value sec from cmdline
parse_input_val:
	xor ax, ax
	xor bx, bx
		
	pop ax                         ; save return addr
	mov dx, ax
	
	;count arg length
	xor ax, ax
    mov al, [ds:0x80]          ; 80h is a location of args count inside PSP segment
	cmp ax, 2                     ; arg char #1 is whitespace, we expect 2-4 chars for input number 0-255
	jl bad_args
	cmp ax, 4
	jg bad_args
	mov si, ax                    ; save args count


	mov bl, [ds:0x82]          ; 82h is 1st byte of args string (after whitespace)
	cmp bl, cr_symbol
	je bad_args
	cmp bl, 0x30                 ; validate decimal number input (allowed 0-9)
	jl bad_args
	cmp bl, 0x39
	jg bad_args 
	sub bl, '0'
	push bx
	
	xor bx, bx
	mov bl, [ds:0x83]
	cmp bl, cr_symbol
	je done_loading_args
	cmp bl, 0x30  
	jl bad_args
	cmp bl, 0x39
	jg bad_args
	sub bl, '0'
	push bx
	
	xor bx, bx
	mov bl, [ds:0x84]
	cmp bl, cr_symbol
	je done_loading_args
	cmp bl, 0x30  
	jl bad_args
	cmp bl, 0x39
	jg bad_args
	sub bl, '0'
	push bx
	
	done_loading_args:
	xor ax, ax
	xor bx, bx
	xor cx, cx
		
	cmp si, 2                     ; arg char 1 is whitespace
	je args_1
	
	cmp si, 3
	je args_2
	
	cmp si, 4
	jmp args_3

	
	args_1:
	pop ax
	mov cx, ax
	
	jmp parsed
	
	
	args_2:
	pop ax
	mov cx, ax
	
	pop ax
	mov bx, ax
	mov ax, 10
	mul bl
	add cx, ax
	
	jmp parsed


	args_3:
	pop ax
	mov cx, ax
	
	pop ax
	mov bx, ax
	mov ax, 10
	mul bl
	add cx, ax
	
	pop ax
	mov bx, ax
	mov ax, 100
	mul bl
	add cx, ax

	parsed:
	cmp cx, 1
	jl bad_args
	cmp cx, 255
	jg bad_args

	push cx                       ; push result
	push dx

	ret
	
;print parsed timeout value as decimal string
print_timeout_val:
	xor si, si

	mov bx, sp
	mov dx, [bx+2]                          ; get timeout val from stack
	
	cmp dx, 10
	jl under_10
	
	cmp dx, 100
	jl under_100
	
	jmp under_1000
	
	
	under_10:
	mov bx, dx
	add bx, '0'
	push bx
	add si, 2                                  ; count occupied stack adresses

	mov dx, sp                               ; print out timeout value digit
	mov cx, 1
	mov bx, STD_OUT
	mov ah, WRITE_FILE
	int 0x21
	
	add sp, si                                ; restore stack to ret addr
	ret
	
	under_100:
	; get 10th place digit
	xor bx, bx
	mov ax, dx
	mov bl, 10
	div bl                                      ; e.g. 25 - now ah=5, al=2
	
	mov di, ax                               ; store val
	
	;print 10th place
	mov bl, al
	add bx, '0'
	push bx
	add si, 2

	mov dx, sp
	mov cx, 1
	mov bx, STD_OUT
	mov ah, WRITE_FILE
	int 0x21
	
	mov ax, di                               ; restore val
	xor bx, bx
	
	;print 1th place
	mov bl, ah
	add bx, '0'
	push bx
	add si, 2

	mov dx, sp
	mov cx, 1
	mov bx, STD_OUT
	mov ah, WRITE_FILE
	int 0x21
	
	add sp, si
	ret
	
	
	under_1000:
	;get 100th place digit
	xor bx, bx
	mov ax, dx
	mov bl, 100
	div bl                                      ; e.g. 251 - now ah=51, al=2
	
	mov di, ax                               ; store val
	
	;print 100th place
	mov bl, al
	add bx, '0'
	push bx
	add si, 2

	mov dx, sp
	mov cx, 1
	mov bx, STD_OUT
	mov ah, WRITE_FILE
	int 0x21
	
	;get 10th place digit
	xor bx, bx
	mov dx, di                               ; restore val e.g. 251 - dh=51, dl=2
	mov al, dh
	mov bl, 10
	div bl                                      ; e.g. 51 - now ah=1, al=5
	
	mov di, ax                               ; store val
		
	;print 10th place digit
	mov bl, al
	add bx, '0'
	push bx
	add si, 2

	mov dx, sp
	mov cx, 1
	mov bx, STD_OUT
	mov ah, WRITE_FILE
	int 0x21
	
	mov ax, di                               ; restore val
	xor bx, bx
	
	;print 1th place digit
	mov bl, ah
	add bx, '0'
	push bx
	add si, 2

	mov dx, sp
	mov cx, 1
	mov bx, STD_OUT
	mov ah, WRITE_FILE
	int 0x21
	
	add sp, si
	ret


section .data
	start_msg db 'Timer started', 0x0d, 0x0a
	start_msg_len equ $ - start_msg
	
	bad_args_msg_1 db 'Usage: timer.com [XXX] - pass X seconds delay', 0x0d, 0x0a
	bad_args_msg_len_1 equ $ - bad_args_msg_1
	
	bad_args_msg_2 db '	decimal number between 1 and 255', 0x0d, 0x0a
	bad_args_msg_len_2 equ $ - bad_args_msg_2
	
	request_msg_start db 'Requested timeout: '
	request_msg_start_len equ $ - request_msg_start
	
	request_msg_end db ' seconds', 0x0d, 0x0a
	request_msg_end_len equ $ - request_msg_end

	STD_OUT equ 1
	WRITE_FILE equ 0x40
	
	cr_symbol equ 0xd
	
	; configured on timer board DIP switch
	; ...1 at the least significant bit of address triggers "shutdown on count finish" mode
	timer_board_address equ 0x221

section .bss 
	; uninitialized data