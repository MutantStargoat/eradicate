; vi:ft=nasm:
	section .text
	bits 32

	global fade_image
fade_image:
	push ebp
	mov ebp, esp
	push ebx

	mov eax, [ebp + 8]
	mov edx, [ebp + 12]
	mov ebx, [ebp + 16]
	call fade_image_

	pop ebx
	pop ebp
	ret

	; void fade_image(uint16_t *dest, uint32_t *src, uint16_t fade)
	;  - dest points to a 16bit 565 framebuffer
	;  - src points to a 32bit RGBX image
	;  - fade is 24.8 fixed point [0, 1]
	; watcom register calling convention arguments: eax, edx, ebx
	global fade_image_
fade_image_:
	push ecx
	push edi

	mov ecx, 640 * 480
	mov edi, eax

	; take fade and duplicate it across all words of mm1
	movd mm1, ebx		; mm1 [00|00|00|VV]
	punpckldq mm1, mm1	; mm1 [00|VV|00|VV]
	packssdw mm1, mm1	; mm1 [VV|VV|VV|VV]
.loop:
	; grab RGB32 pixel and unpack it to zero-extended words in mm0
	movd mm0, [edx]		; mm0 [??|??|?R|GB]
	add edx, 4
	pxor mm7, mm7
	punpcklbw mm0, mm7	; mm0 [0?|0R|0G|0B]
	; multiply by fade and divide by 256 to drop the decimal part
	pmullw mm0, mm1
	psrlw mm0, 10	; 8 for the div + 2 to make them 666, easier 565 packing
	; pack result into 565 in ax
	packuswb mm0, mm0
	movd eax, mm0
	mov ebx, eax
	shr al, 1	; blue in position [........|00RRRRRR|00GGGGGG|000BBBBB]
	xor bl, bl
	shr bx, 3
	xor ah, ah
	or ax, bx	; green in position ...|00RRRRRR|00000GGG|GGGBBBBB]
	shr ebx, 6
	and ebx, 0f800h
	or eax, ebx	; done [RRRRRGGG|GGGBBBBB]
	mov [edi], ax
	add edi, 2

	dec ecx
	jnz .loop

	emms		; clear fpu state

	pop edi
	pop ecx
	ret
