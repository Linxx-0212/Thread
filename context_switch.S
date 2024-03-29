/**
 * Context switches to new_context from old_context.
 *
 * Saves the current execution context into the grn_context structure pointed to
 * by the first paramter and restores the execution context pointed to by the
 * second paramter. As such, when this function returns, it jumps to the address
 * at the top of the stack (pointed to by %rsp).
 *
 * This function has the following signature in C:
 * grn_context_switch(grn_context *old_context, grn_context *new_context)
 *
 * @param old_context pointer to where to save the current context
 * @param new_context pointer to the context to restore
 */
.globl grn_context_switch
grn_context_switch:
  // FIXME: Save the old context, restore the new.

  #save
  mov %rsp, (%rdi)
  mov %r15, 0x8(%rdi)
  mov %r14, 0x10(%rdi)
  mov %r13, 0x18(%rdi)
  mov %r12, 0x20(%rdi)
  mov %rbx, 0x28(%rdi)
  mov %rbp, 0x30(%rdi)

  #restore
  mov (%rsi), %rsp
  mov 0x8(%rsi), %r15
  mov 0x10(%rsi), %r14
  mov 0x18(%rsi), %r13
  mov 0x20(%rsi), %r12
  mov 0x28(%rsi), %rbx
  mov 0x30(%rsi), %rbp


  ret

/**
 * Initial function implicitly executed by a thread.
 *
 * This function should be jumped into implicitly by the green threads library.
 * It expects a function pointer at the top of the stack and subsequently calls
 * that function. When that function returns, this function calls grn_exit. It
 * does not expect grn_exit to return. If it does, this function loops
 * infinitely.
 *
 * @param fn [expected at top of stack] a function to call
 */
.globl start_thread
start_thread:
  push    %rbp
  mov     %rsp, %rbp
  mov     0x8(%rbp), %r11
  callq   *%r11
  callq   _grn_exit
loop:
  jmp     loop
