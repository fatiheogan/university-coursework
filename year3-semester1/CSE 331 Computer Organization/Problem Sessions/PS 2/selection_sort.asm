.data
array:  .word 9, 5, 17, 1, 2, 11    # array elements
n:      .word 6
msg:    .asciiz "Sorted array:\n"

.text
.globl main

main:
    la   $t0, array         # $t0 = array's base address
    lw   $t1, n             # $t1 = number of elements
    addi $t1, $t1, -1       # limit = n - 1
    add  $t2, $zero, $zero  # i = 0

outer_loop:
    slt  $t9, $t1, $t2      # if t1 < t2 (i >= n-1) => end
    bne  $t9, $zero, done_outer

    add  $t3, $t2, $zero    # index of smallest element = i
    addi $t4, $t2, 1        # j = i + 1

inner_loop:
     lw   $t8, n
     slt  $t9, $t4, $t8      # $t9 = 1 if j < n
     beq  $t9, $zero, check_swap  # exit inner if j >= n

    # Load A[min_index]
    sll  $t6, $t3, 2 #shift left by 2 for next index
    add  $t6, $t6, $t0 #t6 = A[min_index]'s address 
    lw   $t7, 0($t6) #t7 = A[min_index]
    

    # Load A[j]
    sll  $t8, $t4, 2 #shift left by 2 for next index
    add  $t8, $t8, $t0 #t8 = A[j]'s address 
    lw   $t9, 0($t8) #t9 = A[j]

    # Compare A[j] < A[min_index]
    slt  $s0, $t9, $t7 # if A[j] > A[min_index] don't update
    beq  $s0, $zero, dont_update

    add  $t3, $t4, $zero    # min_index = j 

dont_update:
    addi $t4, $t4, 1 # j = j + 1
    j    inner_loop

check_swap: #t2 = position we are sorting, t3 = minimum index
    beq  $t3, $t2, dont_swap   # If min_index == i, then there’s nothing to swap

    # Swap A[i] and A[min_index]
    sll  $t6, $t2, 2 # multiply i by 4 
    add  $t6, $t6, $t0 
    lw   $t7, 0($t6) # t7 = A[i]

    sll  $t8, $t3, 2
    add  $t8, $t8, $t0
    lw   $t9, 0($t8) #t9 = A[min_index]

    sw   $t9, 0($t6) # A[i] = A[min_index]
    sw   $t7, 0($t8) # A[min_index] = A[i]

dont_swap:
    addi $t2, $t2, 1
    j    outer_loop

done_outer:
    # Print header
    li   $v0, 4
    la   $a0, msg
    syscall

    # Print sorted array
    la   $t0, array
    lw   $t1, n
    add  $t2, $zero, $zero

print_loop:
    slt  $t9, $t2, $t1   # $t9 = 1 if t2 < t1
    beq  $t9, $zero, exit  # exit when i >= n

    sll  $t3, $t2, 2
    add  $t3, $t3, $t0
    lw   $a0, 0($t3)
    li   $v0, 1
    syscall

    # print space
    li   $v0, 11
    addi $a0, $zero, 32
    syscall

    addi $t2, $t2, 1
    j    print_loop

exit:
    li   $v0, 10
    syscall
