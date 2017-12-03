	.arch msp430
	.p2align 1,0

	.weak buzzer

	.text

	.extern p1collision
	.extern p2collision
	.extern fcollision

	.global change_sound
change_sound:
	call #p1collision
	mov &buzzer, r12
	mov r12, r6
	cmp #1, r6
	jz play_sound1
	call #p2collision
	mov r12, r6
	cmp #1, r6
	jz play_sound2
	call #fcollision
	mov r12, r6
	cmp #1, r6
	jz play_sound3


play_sound1:			;state when paddle 1 has collision
	mov #2000, r12		;period = 2000
	call #set_pd		;set period to 2000
	mov #1, &buzzer		;buzzer = 1
	jmp done


play_sound2:			;state when paddle 2 has collision
	mov #2000, r12		;period = 2000
	call #set_pd		;set period to 2000
	mov #1, &buzzer		;buzzer = 1
	jmp done


play_sound3:			;state when fence has collision
	mov #7000, r12		;period = 7000
	call #set_pd		;set period to 7000
	mov #1, &buzzer		;buzzer = 1
	jmp done


done:
	mov #0, &buzzer		;buzzer = 0
