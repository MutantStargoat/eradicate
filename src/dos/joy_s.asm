; vi:ft=nasm:
	section .text
	bits 32

	global _have_joy
	global _joy_bnstate
	global _joy_bndiff
	global _joy_bnpress
	global _joy_pos

JOY_PORT equ 201h

	global joy_detect_
joy_detect_:
	; TODO
	mov eax, 1
	mov [_have_joy], eax
	ret

	global joy_update_
joy_update_:
	pusha
	cli
	mov bl, 2	; pending axes
	xor eax, eax
	not eax		; init previous to ff
	xor ecx, ecx	; init timeout counter
	mov dx, JOY_PORT
	out dx, al
.count:
	in al, dx
	xor ah, al	; detect change from previous
	test ah, 1
	jz .axis0_notdone
	mov [rawcnt], ecx
	dec bl
	jz .done
.axis0_notdone:
	test ah, 2
	jz .axis1_notdone
	mov [rawcnt + 4], ecx
	dec bl
	jz .done
.axis1_notdone:
	mov ah, al	; update previous
	inc ecx
	jnz .count	; is waiting for rollover to abort too much?
	mov dword [_have_joy], 0	; joystick timed out, drop it
	sti
	popa
	ret
.done:
	sti
	not eax
	and eax, 0f0h
	; TODO dpad
	
	mov ecx, [_joy_bnstate]
	xor ecx, eax
	mov [_joy_bndiff], ecx
	and ecx, eax
	mov [_joy_bnpress], ecx
	mov [_joy_bnstate], eax
	
	popa
	ret

	section .bss
	align 4
rawcnt resd 2
_have_joy resd 1
_joy_bnstate resd 1
_joy_bndiff resd 1
_joy_bnpress resd 1
_joy_pos resw 2
