	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 15, 0
	.globl	_fun                            ; -- Begin function fun
	.p2align	2
_fun:                                   ; @fun
	.cfi_startproc
; %bb.0:                                ; %entry_fun
	stp	x29, x30, [sp, #-16]!           ; 16-byte Folded Spill
	mov	x29, sp
	sub	sp, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	mov	w8, #1
	stur	w8, [x29, #-4]
LBB0_1:                                 ; %for_in0
                                        ; =>This Inner Loop Header: Depth=1
	stur	w8, [x29, #-8]
	mov	w8, w8
	cmp	w8, #999
	b.gt	LBB0_3
; %bb.2:                                ; %for_body0
                                        ;   in Loop: Header=BB0_1 Depth=1
	ldur	w8, [x29, #-8]
	lsl	w8, w8, #1
	b	LBB0_1
LBB0_3:                                 ; %for_end0
	mov	x8, sp
	sub	x0, x8, #16
	mov	sp, x0
	mov	w9, #10
	sturh	w9, [x8, #-16]
	bl	_putstr
	ldur	w0, [x29, #-8]
	bl	_putint
	mov	sp, x29
	ldp	x29, x30, [sp], #16             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:                                ; %entry_main
	stp	x22, x21, [sp, #-48]!           ; 16-byte Folded Spill
	stp	x20, x19, [sp, #16]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	sub	sp, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	mov	x9, #12594
	mov	w8, #10
	movk	x9, #14131, lsl #16
	sub	x0, x29, #42
	movk	x9, #13363, lsl #32
	movk	x9, #14133, lsl #48
	sturh	w8, [x29, #-34]
	stur	x9, [x29, #-42]
	bl	_putstr
Lloh0:
	adrp	x8, _ZERO@PAGE
	adrp	x20, _var2@PAGE
	adrp	x21, _var3@PAGE
Lloh1:
	adrp	x10, _ONE@PAGE
Lloh2:
	ldr	w9, [x8, _ZERO@PAGEOFF]
	ldr	w8, [x20, _var2@PAGEOFF]
	ldr	w11, [x21, _var3@PAGEOFF]
Lloh3:
	ldr	w19, [x10, _ONE@PAGEOFF]
	add	w8, w9, w8
	sub	w10, w11, w19
	cmp	w8, w10
	b.ne	LBB1_3
; %bb.1:                                ; %entry_main
	cbz	w19, LBB1_3
; %bb.2:                                ; %and_true2
	mov	w8, #1
	sturb	w8, [x29, #-43]
	b	LBB1_4
LBB1_3:                                 ; %and_false2
	sturb	wzr, [x29, #-43]
LBB1_4:                                 ; %and_end2
	ldurb	w8, [x29, #-43]
	cmp	w8, #1
	b.ne	LBB1_16
; %bb.5:                                ; %or_entry4
	sub	x8, sp, #16
	mov	sp, x8
	cbnz	w9, LBB1_11
; %bb.6:                                ; %and_entry5
	sub	x10, sp, #16
	mov	sp, x10
	cbnz	w9, LBB1_9
; %bb.7:                                ; %and_right5
	ldr	w9, [x20, _var2@PAGEOFF]
	add	w9, w19, w9
	cmn	w9, #1
	b.pl	LBB1_9
; %bb.8:                                ; %and_true5
	mov	w9, #1
	strb	w9, [x10]
	b	LBB1_10
LBB1_9:                                 ; %and_false5
	strb	wzr, [x10]
LBB1_10:                                ; %and_end5
	ldrb	w9, [x10]
	cmp	w9, #1
	b.ne	LBB1_28
LBB1_11:                                ; %or_true4
	mov	w9, #1
	strb	w9, [x8]
LBB1_12:                                ; %or_end4
	ldrb	w8, [x8]
	cmp	w8, #1
	b.ne	LBB1_14
; %bb.13:                               ; %if_body3
	mov	x8, sp
	sub	x0, x8, #16
	mov	sp, x0
	mov	x9, #21061
	movk	x9, #20306, lsl #16
	movk	x9, #8530, lsl #32
	movk	x9, #10, lsl #48
	stur	x9, [x8, #-16]
	b	LBB1_15
LBB1_14:                                ; %else_body3
	mov	x8, sp
	sub	x0, x8, #16
	mov	sp, x0
	mov	x11, #28225
	mov	w10, #29541
	movk	x11, #8292, lsl #16
	mov	w9, #10
	movk	x11, #30067, lsl #32
	movk	w10, #8563, lsl #16
	movk	x11, #25443, lsl #48
	sturh	w9, [x8, #-4]
	stur	w10, [x8, #-8]
	stur	x11, [x8, #-16]
LBB1_15:                                ; %if_end3
	bl	_putstr
LBB1_16:                                ; %or_entry7
	sub	x8, sp, #16
	mov	sp, x8
	ldr	w9, [x21, _var3@PAGEOFF]
	cmp	w9, #3
	b.ne	LBB1_18
; %bb.17:                               ; %or_right7
	ldr	w9, [x20, _var2@PAGEOFF]
	cmp	w9, #2
	b.ne	LBB1_19
LBB1_18:                                ; %or_true7
	mov	w9, #1
	strb	w9, [x8]
	b	LBB1_20
LBB1_19:                                ; %or_false7
	strb	wzr, [x8]
LBB1_20:                                ; %or_end7
	ldrb	w8, [x8]
	cmp	w8, #1
	b.ne	LBB1_27
; %bb.21:                               ; %or_entry9
	sub	x8, sp, #16
	mov	sp, x8
	cmp	w19, #0
	ldr	w10, [x21, _var3@PAGEOFF]
	cinc	w9, w19, lt
	ldr	w11, [x20, _var2@PAGEOFF]
	and	w9, w9, #0xfffffffe
	sub	w9, w19, w9
	add	w9, w9, w10
	add	w9, w9, w11
	sub	w9, w9, #5
	cmp	w9, #101
	b.lt	LBB1_24
; %bb.22:                               ; %or_entry9
	cbnz	w19, LBB1_24
; %bb.23:                               ; %or_false9
	strb	wzr, [x8]
	b	LBB1_25
LBB1_24:                                ; %or_true9
	mov	w9, #1
	strb	w9, [x8]
LBB1_25:                                ; %or_end9
	ldrb	w8, [x8]
	cmp	w8, #1
	b.ne	LBB1_27
; %bb.26:                               ; %if_body8
	mov	x8, sp
	sub	x0, x8, #16
	mov	sp, x0
	mov	x10, #29263
	mov	w9, #10
	movk	x10, #28704, lsl #16
	movk	x10, #29537, lsl #32
	movk	x10, #8563, lsl #48
	sturh	w9, [x8, #-8]
	stur	x10, [x8, #-16]
	bl	_putstr
LBB1_27:                                ; %if_end6
	mov	x8, sp
	sub	x0, x8, #16
	mov	sp, x0
	mov	x11, #25940
	mov	w10, #25443
	movk	x11, #29811, lsl #16
	mov	w9, #8563
	movk	x11, #8241, lsl #32
	movk	w10, #29541, lsl #16
	movk	x11, #30035, lsl #48
	sturb	wzr, [x8, #-2]
	sturh	w9, [x8, #-4]
	stur	w10, [x8, #-8]
	stur	x11, [x8, #-16]
	bl	_putstr
	bl	_fun
	bl	_fun
	bl	_fun
	bl	_fun
	bl	_fun
	bl	_fun
	mov	w0, wzr
	sub	sp, x29, #32
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp], #48             ; 16-byte Folded Reload
	ret
LBB1_28:                                ; %or_false4
	strb	wzr, [x8]
	b	LBB1_12
	.loh AdrpLdr	Lloh1, Lloh3
	.loh AdrpLdr	Lloh0, Lloh2
	.cfi_endproc
                                        ; -- End function
	.globl	_getchar                        ; -- Begin function getchar
	.p2align	2
_getchar:                               ; @getchar
	.cfi_startproc
; %bb.0:                                ; %getchar_entry
	sub	sp, sp, #32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	.cfi_def_cfa_offset 32
	.cfi_offset w30, -8
	.cfi_offset w29, -16
Lloh4:
	adrp	x0, l_.str2@PAGE
	add	x8, sp, #15
Lloh5:
	add	x0, x0, l_.str2@PAGEOFF
	str	x8, [sp]
	bl	_scanf
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	ldrsb	w0, [sp, #15]
	add	sp, sp, #32
	ret
	.loh AdrpAdd	Lloh4, Lloh5
	.cfi_endproc
                                        ; -- End function
	.globl	_getint                         ; -- Begin function getint
	.p2align	2
_getint:                                ; @getint
	.cfi_startproc
; %bb.0:                                ; %getint_entry
	sub	sp, sp, #32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	.cfi_def_cfa_offset 32
	.cfi_offset w30, -8
	.cfi_offset w29, -16
Lloh6:
	adrp	x0, l_.str1@PAGE
	add	x8, sp, #12
Lloh7:
	add	x0, x0, l_.str1@PAGEOFF
	str	x8, [sp]
	bl	_scanf
LBB3_1:                                 ; %clear_entry
                                        ; =>This Inner Loop Header: Depth=1
	bl	_getchar
	cmp	w0, #10
	b.ne	LBB3_1
; %bb.2:                                ; %clear_out
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	ldr	w0, [sp, #12]
	add	sp, sp, #32
	ret
	.loh AdrpAdd	Lloh6, Lloh7
	.cfi_endproc
                                        ; -- End function
	.globl	_putint                         ; -- Begin function putint
	.p2align	2
_putint:                                ; @putint
	.cfi_startproc
; %bb.0:                                ; %putint_entry
	sub	sp, sp, #32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	.cfi_def_cfa_offset 32
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	mov	w8, w0
Lloh8:
	adrp	x0, l_.str1@PAGE
Lloh9:
	add	x0, x0, l_.str1@PAGEOFF
	str	x8, [sp]
	bl	_printf
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #32
	ret
	.loh AdrpAdd	Lloh8, Lloh9
	.cfi_endproc
                                        ; -- End function
	.globl	_putchar                        ; -- Begin function putchar
	.p2align	2
_putchar:                               ; @putchar
	.cfi_startproc
; %bb.0:                                ; %putchar_entry
	sub	sp, sp, #32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	.cfi_def_cfa_offset 32
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	mov	w8, w0
Lloh10:
	adrp	x0, l_.str2@PAGE
Lloh11:
	add	x0, x0, l_.str2@PAGEOFF
	str	x8, [sp]
	bl	_printf
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #32
	ret
	.loh AdrpAdd	Lloh10, Lloh11
	.cfi_endproc
                                        ; -- End function
	.globl	_putstr                         ; -- Begin function putstr
	.p2align	2
_putstr:                                ; @putstr
	.cfi_startproc
; %bb.0:                                ; %putstr_entry
	stp	x29, x30, [sp, #-16]!           ; 16-byte Folded Spill
	.cfi_def_cfa_offset 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	bl	_printf
	ldp	x29, x30, [sp], #16             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__const
	.globl	_ONE                            ; @ONE
	.p2align	2
_ONE:
	.long	1                               ; 0x1

	.globl	_ZERO                           ; @ZERO
	.p2align	2
_ZERO:
	.long	0                               ; 0x0

	.section	__DATA,__data
	.globl	_var2                           ; @var2
	.p2align	2
_var2:
	.long	2                               ; 0x2

	.globl	_var3                           ; @var3
	.p2align	2
_var3:
	.long	3                               ; 0x3

	.section	__TEXT,__const
l_.str1:                                ; @.str1
	.asciz	"%d"

l_.str2:                                ; @.str2
	.asciz	"%c"

.subsections_via_symbols
