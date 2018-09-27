#include "i8042.h"

.intel_syntax

.global _read_key_code_asm,  _scanned_key_code_asm

.bss
.lcomm _scanned_key_code_asm, 32

.data

.text

_read_key_code_asm:
xor eax, eax
xor ebx, ebx
inb KBD_STATUS_REG
add bl, TIMEOUT_ERR
add bl, PARITY_ERR
add bl, AUX_FLAG
and bl, al  /*test parity, timeout and aux flag errors*/
jnz error
xor ebx, ebx
add bl, OUF_FLAG
and bl, al  /*test if there is anything to read*/
jnz data_ok

error:
xor eax, eax
mov _scanned_key_code_asm, eax  /*if there is an error return 0*/
jmp end

data_ok:
inb OUT_BUF
mov _scanned_key_code_asm, eax  /*if everything is OK return the scanned key code */

end:
ret
