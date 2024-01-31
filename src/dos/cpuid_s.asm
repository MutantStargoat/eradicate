	section .text use32
	bits 32
; foo_ are watcom functions, _foo are djgpp functions

F_ID equ 0x200000

	global read_cpuid
	global _read_cpuid
	global read_cpuid_
read_cpuid_:
	push eax
	call check_cpuid
	pop eax
	jnc read_cpuid_nocheck
	mov eax, -1
	ret

_read_cpuid:
read_cpuid:
	call check_cpuid
	mov eax, [esp + 4]
	jnc read_cpuid_nocheck
	mov eax, -1
	ret

	; determine if cpuid is available. avail: cf=0, not avail: cf=1
check_cpuid:
	pushf
	pop eax
	mov edx, eax	; keep a copy of the original eflags in edx
	xor eax, F_ID
	push eax
	popf
	pushf
	pop eax
	cmp eax, edx
	clc
	jnz .noerr
	stc
.noerr:	ret

	; enter with the cpuid_info structure pointer in eax
read_cpuid_nocheck:
	push ebp
	mov ebp, esp
	push ebx
	push edi
	push esi
	push eax	; save the original struct pointer
	sub esp, 8
	mov edi, eax	; struct pointer -> edi

	; clear struct
	cld
	push edi
	mov ecx, (32+48)/4
	xor eax, eax
	rep stosd
	pop edi

	xor eax, eax
	mov [esp], eax	; current index
	cpuid

	mov [edi], eax		; maxidx
	; clamp to the size of our cpuid_info structure
	cmp eax, 1
	jbe .skipclamp
	mov eax, 1
.skipclamp:
	mov [esp + 4], eax	; maximum index

	mov [edi + 4], ebx	; vendor name
	mov [edi + 8], edx
	mov [edi + 12], ecx
	add edi, 16

.loop:	mov eax, [esp]
	inc eax
	cmp eax, [esp + 4]
	ja .loopend
	mov [esp], eax
	cpuid
	mov [edi], eax
	mov [edi + 4], ebx
	mov [edi + 8], edx
	mov [edi + 12], ecx
	add edi, 16
	jmp .loop
.loopend:
	; try to retrieve the brand string (avail on P4 or newer)
	mov eax, 80000000h
	cpuid
	test eax, 80000000h
	jz .done	; no extended cpuid functions
	cmp eax, 80000004h
	jb .done	; no brand string available

	; brand string available
	mov esi, esp		; save esp to esi
	mov esp, [esp + 8]	; esp <- original struct pointer
	add esp, 32+48		; offset to end of brandstr
	mov eax, 80000004h
	cpuid
	push edx
	push ecx
	push ebx
	push eax
	mov eax, 80000003h
	cpuid
	push edx
	push ecx
	push ebx
	push eax
	mov eax, 80000002h
	cpuid
	push edx
	push ecx
	push ebx
	push eax
	mov esp, esi	; done restore esp

.done:	add esp, 8
	pop eax
	pop esi
	pop edi
	pop ebx
	pop ebp
	xor eax, eax
	ret
	
; vi:ft=nasm:
