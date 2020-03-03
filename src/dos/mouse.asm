; vi:set filetype=nasm:
; foo_ are watcom functions, _foo are djgpp functions

QUERY equ 0
SHOW equ 1
HIDE equ 2
READ equ 3
WRITE equ 4
PIXRATE equ 15
XLIM equ 7
YLIM equ 8

PUSHA_EAX_OFFS equ 28
PUSHA_ECX_OFFS equ 20
PUSHA_EDX_OFFS equ 16

	section .text
	bits 32

; int have_mouse(void)
	global have_mouse_
	global _have_mouse
have_mouse_:
_have_mouse:
	pusha
	mov ax, QUERY
	int 0x33
	and eax, 0xffff
	mov [esp + PUSHA_EAX_OFFS], eax
	popa
	ret

; void show_mouse(int show)
	global show_mouse_
show_mouse_:
	pusha
	test ax, ax
	mov ax, HIDE
	jz .skip
	mov ax, SHOW
.skip:	int 0x33
	popa
	ret

	global _show_mouse
_show_mouse:
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	mov ax, [ebp + 8]
	test ax, ax
	mov ax, HIDE
	jz .skip
	mov ax, SHOW
.skip:	int 0x33
	pop edi
	pop esi
	pop ebx
	pop ebp
	ret

; int read_mouse(int *xp, int *yp)
	global read_mouse_
read_mouse_:
	pusha
	mov esi, eax	; xp
	mov edi, edx	; yp
	mov ax, READ
	int 0x33
	xor eax, eax
	and ecx, 0xffff
	and edx, 0xffff
	mov ax, bx
	mov [esp + PUSHA_EAX_OFFS], eax
	mov [esi], ecx
	mov [edi], edx
	popa
	ret

	global _read_mouse
_read_mouse:
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	mov ax, READ
	int 0x33
	xor eax, eax
	mov ax, bx
	and ecx, 0xffff
	mov ebx, [ebp + 8]
	mov [ebx], ecx
	and edx, 0xffff
	mov ebx, [ebp + 12]
	mov [ebx], edx
	pop edi
	pop esi
	pop ebx
	pop ebp
	ret

; void set_mouse(int x, int y)
	global set_mouse_
set_mouse_:
	pusha
	mov cx, ax
	mov ax, WRITE
	int 0x33
	popa
	ret
	
	global _set_mouse
_set_mouse:
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	mov ax, WRITE
	mov cx, [ebp + 8]
	mov dx, [ebp + 12]
	int 0x33
	pop edi
	pop esi
	pop ebx
	pop ebp
	ret

; void set_mouse_limits(int xmin, int ymin, int xmax, int ymax)
	global set_mouse_limits_
set_mouse_limits_:
	pusha
	mov cx, ax
	mov dx, bx
	mov ax, XLIM
	int 0x33
	mov ax, YLIM
	mov cx, [esp + PUSHA_EDX_OFFS]
	mov dx, [esp + PUSHA_ECX_OFFS]
	int 0x33
	popa
	ret

	global _set_mouse_limits
_set_mouse_limits:
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	mov ax, XLIM
	mov cx, [ebp + 8]
	mov dx, [ebp + 16]
	int 0x33
	mov ax, YLIM
	mov cx, [ebp + 12]
	mov dx, [ebp + 20]
	int 0x33
	pop edi
	pop esi
	pop ebx
	pop ebp
	ret

; void set_mouse_rate(int xrate, int yrate)
	global set_mouse_rate_
set_mouse_rate_:
	pusha
	mov cx, ax
	mov ax, PIXRATE
	int 0x33
	popa
	ret

	global _set_mouse_rate
_set_mouse_rate:
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	mov ax, PIXRATE
	mov cx, [esp + 4]
	mov dx, [esp + 8]
	int 0x33
	pop edi
	pop esi
	pop ebx
	pop ebp
	ret
