.code

GetPEB proc
	mov rax, gs:[60h]
	ret
GetPEB endp

CheckDebugger proc
	xor eax, eax
	call GetPEB
	movzx eax, byte ptr [rax+2h] ; PEB->BeingDebugged
	ret
CheckDebugger endp
end
