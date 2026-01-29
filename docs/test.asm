: move_rax_by_one

PUSH 1
PUSHR RAX
ADD
POPR RAX

RET


: move_scope

PUSHR RBX
POPM [RAX]

PUSHR RAX
POPR  RBX

CALL move_rax_by_one

RET



: ret_scope
PUSHR RBX
POPR RAX

PUSHM [RBX]
POPR RBX

RET



: set_rcx_offset
PUSHR RBX
ADD
POPR RCX

RET



: get_rax_by_offset
CALL set_rcx_offset
PUSHM [RCX]

RET



: func
POPM [RAX]
CALL move_rax_by_one

PUSH 1
PUSH 1
CALL get_rax_by_offset

JNE endif_1

PUSH 1
CALL get_rax_by_offset
OUT 

CALL ret_scope

PUSH 0
RET

: endif_1

PUSH 1
CALL get_rax_by_offset
PUSH 1
SUB

CALL move_scope
CALL func

PUSH 1
CALL get_rax_by_offset
OUT

CALL ret_scope

PUSH 0
RET


MAIN

PUSH 0
POPR RAX

PUSH 0
POPR RBX

PUSH 0
POPR RCX

PUSH 10
CALL move_scope
CALL func

; CALL ret_scope

PUSH 0

HLT