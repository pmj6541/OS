<시스템 콜을 위한 수정>
sysproc.c
syscall.c
syscall.h
user.h
usys.S
<Test File 및 Makefile 수정>
Makefile
checkpage.c : fork 및 getNumFreePages를 통해 free page num 을 관측하는 함수
<과제 내용>
mmu.h : trap.c 에서 사용하는 T_PGFLT 추가 정의
vm.c : page fault handler 구현, copyuvm수정
defs.h : 구현한 함수 사용을 위해 등록.
trap.c : pagefault 발생 시 pagefault함수 호출
kalloc.c : get, inc, dec_refcount 함수 구현, reference counter 적용,
num_free_pages 변수 추가.