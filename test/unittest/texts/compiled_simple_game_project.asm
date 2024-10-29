option casemap :none
.code
?CopyMemory proc
	push rsi
	push rdi
	push rcx
	mov rsi, rcx
	mov rdi, rdx
	mov rcx, r8
?CopyMemory?L1:
	mov al, byte ptr [rsi]
	mov byte ptr [rdi], al
	inc rsi
	inc rdi
	loop ?CopyMemory?L1
	pop rcx
	pop rdi
	pop rsi
	ret
?CopyMemory endp
.data
?0 byte 67, 108, 97, 115, 115, 78, 97, 109, 101, 84, 101, 115, 116, 237, 133, 140, 236, 138, 164, 237, 138, 184, 0
?1 byte 67, 108, 97, 115, 115, 78, 97, 109, 101, 84, 101, 115, 116, 0
?2 byte 87, 105, 110, 100, 111, 119, 78, 97, 109, 101, 84, 101, 115, 116, 237, 133, 140, 236, 138, 164, 237, 138, 184, 0
?3 byte 87, 105, 110, 100, 111, 119, 78, 97, 109, 101, 84, 101, 115, 116, 0
?4 byte 72, 101, 108, 108, 111, 32, 87, 111, 114, 108, 100, 33, 10, 0
includelib "kernel32.lib"
int32 typedef dword
uint32 typedef dword
address typedef qword
bool typedef byte
false bool 0
true bool 1
includelib "libcmt.lib"
printf PROTO format:address, args:VARARG
includelib "SimpleApplication.lib"
SimpleApplication_InitializeWithUTF8 PROTO className:address, windowName:address
SimpleApplication_InitializeWithAnsi PROTO className:address, windowName:address
SimpleApplication_DeInitializeAll PROTO
SimpleApplication_Update PROTO
.code
main proc
	push rbp
	sub rsp, 128
	sub rsp, 32
	mov r8, sizeof ?0
	lea rdx, [rsp + 32]
	lea rcx, [?0]
	call ?CopyMemory
	add rsp, 32
	sub rsp, 32
	mov r8, sizeof ?1
	lea rdx, [rsp + 64]
	lea rcx, [?1]
	call ?CopyMemory
	add rsp, 32
	sub rsp, 32
	mov r8, sizeof ?2
	lea rdx, [rsp + 96]
	lea rcx, [?2]
	call ?CopyMemory
	add rsp, 32
	sub rsp, 32
	mov r8, sizeof ?3
	lea rdx, [rsp + 128]
	lea rcx, [?3]
	call ?CopyMemory
	add rsp, 32
	sub rsp, 32
	lea rdx, [rsp + 96]
	lea rcx, [rsp + 32]
	call SimpleApplication_InitializeWithUTF8
	add rsp, 32
	sub rsp, 32
	lea rdx, [rsp + 128]
	lea rcx, [rsp + 64]
	call SimpleApplication_InitializeWithAnsi
	add rsp, 32
	sub rsp, 32
	lea rcx, [?4 + 32]
	call printf
	add rsp, 32
?main_L0:
	sub rsp, 32
	call SimpleApplication_Update
	add rsp, 32
	cmp eax, 0
	je ?main_L2
?main_L1:
	jmp ?main_L0
?main_L2:
	sub rsp, 32
	call SimpleApplication_DeInitializeAll
	add rsp, 32
	add rsp, 128
	pop rbp
	ret
main endp
END