.intel_syntax noprefix
.globl main
main:
  push 10
  push 20
  push 5
  pop rdi
  pop rax
  cqo
  idiv rdi
  push rax
  pop rdi
  pop rax
  add rax, rdi
  push rax
  pop rax
  ret
