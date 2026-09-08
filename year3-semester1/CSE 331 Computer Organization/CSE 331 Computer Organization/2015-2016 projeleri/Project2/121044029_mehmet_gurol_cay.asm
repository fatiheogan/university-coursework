.data
welcomeMsg:		.asciiz "\n----------- A NEW COURT -------------\n"
jugeNameMsg:		.asciiz "1. The name of the judge: "
juryNumber:		.asciiz "2. The number of people in the jury: "
errorMsg: 		.asciiz "THERE IS NO JUDGE NAMED AS"
judgeVillain: 		.asciiz "Villain\n"
judgeMiscreant: 	.asciiz "Miscreant\n"

giveJurryFee:		.asciiz	"Give JURY FEE to "
print3:			.asciiz " people and no fee to the judge. (Rule 3)\n"
print2:			.asciiz " people and FREEDOM WITH JURY FEE to the judge. (Rule 2)\n"
print1:			.asciiz " people and ULTIMATE FREEDOM FEE to the judge. (Rule 1)\n" 
		
printSpend:		.asciiz	"You have to spend "
printSpend2:		.asciiz "NL at least.\n"
printSumSign:		.asciiz " + "
printEqualSign:		.asciiz " = "
printMultSign:		.asciiz " * "

ultFreeFeeMisc:		.word	8000		# Ultimate Freedom Fee for Judge Miscreant
ultFreeFeeVil:		.word	6000		# Ultimate Freedom Fee for Judge Villain	

freeJuryFeeMisc:	.word	3000		# Freedom with Jury Fee for Judge Miscreant
freeJuryFeeVil:		.word	4000		# Freedom with Jury Fee for Judge Villain
juryFee:		.word 	200		# Each jury member Fee 

judgeName:		.space 	20

.text
main:
	li $v0, 4
	la $a0, welcomeMsg
	syscall
	la $a0, jugeNameMsg
	syscall
	#read data
	li $v0, 8
	la $a0, judgeName
	li $a1, 20	#get space on stack
	move $s1, $a0
	syscall
	
	#check for is judge Villian
	la $s2,judgeVillain
	jal controlJudge
	bnez $v0,match		# if( judgeVillain != 0)
	#check to judge is Miscreant
	la $s2,judgeMiscreant 
	jal controlJudge
	bnez $v0,match		# if( judgeMiscreant != 0)
	j notMatch
	
match:	
	li $v0, 4
	la $a0, juryNumber
	syscall
	li $v0, 5		#get the jury number from user
	syscall
	move $s7,$v0
	
	la $s2,judgeVillain
	jal controlJudge
	bnez $v0,computeVillain	
	la $s2,judgeMiscreant
	jal controlJudge
	bnez $v0,computeMiscreant
	
notMatch:
	li $v0, 4
	la $a0,errorMsg
	syscall
	la $a0,judgeName
	j main

controlJudge:
	add $t1,$zero,$s1
	add $t2,$zero,$s2
	
loop:	lb $t3($t1)
	lb $t4($t2)
	beqz $t3,isEnd
	beqz $t4,notEqual
	bne $t3,$t4, notEqual
	addi $t1,$t1, 1
	addi $t2,$t2, 1
	j loop
equal:
	addi $v0, $v0, 1
	j endFunc
isEnd:	
	beqz $t4,equal
notEqual:
	add $v0,$zero,$zero
	j endFunc
endFunc:
	jr $ra
	
computeVillain:
	lw $t1, juryFee
	lw $t2, ultFreeFeeVil
	lw $t5, freeJuryFeeVil
	
	move $s2,$ra		# save the return adress
	
	jal rule1
	move $s4,$v0		# this is first rule's result
	jal rule2	
	move $s5,$v0		# this is secont rule's result
	jal rule3 
	move $s6, $v0		# this is third rule's result
	
	lw $t9, ultFreeFeeVil
	jal compareResult
	
computeMiscreant:
	lw $t1,juryFee
	lw $t2,ultFreeFeeMisc
	lw $t5,freeJuryFeeMisc
	
	move $s2, $ra
	
	jal rule1
	move $s4,$v0		# this is first rule's result
	jal rule2	
	move $s5,$v0		# this is secont rule's result
	jal rule3  
	move $s6, $v0		# this is third rule's result
	
	lw $t9,ultFreeFeeMisc
	jal compareResult

rule1:
	mul $t3,$s7,$t1		# $s7 == juryNumber ($t3 = juryNumber * Each jury member Fee)
	mul $t3,$t3,10		# $t3 = $t3 * %10
	div $t3,$t3,100		# $t3 = $t3 * %10
	add $t3,$t3,$t2		# $t3 = $t3 + Ultimate Freedom Fee
	mfhi $t4		# this is remainder from division
	move $t8, $ra
	bnez $t4,onceIncre  	# if remainder is exist, result will increase once
	move $v0, $t3
	jr $t8
rule2:
	mul $t3,$s7,$t1		# $s7 == juryNumber ($t3 = juryNumber * Each jury member Fee)
	mul $t3,$t3,40		# $t3 = $t3 * %40
	div $t3,$t3,100		# $t3 = $t3 * %40	
	add $t3,$t3,$t5		# $t3 = $t3 + Freedom with Jury Fee
	mfhi $t4		# this is remainder from division
	move $t8, $ra
	bnez $t4, onceIncre	# if remainder is exist, result will increase once
	move $v0, $t3
	jr $t8
rule3:
	mul $t2,$t1,$s7		# $t2 = Each jury member Fee * Jury Number
	mul $t2,$t2,80		# $t2 = $t2 * %50
	div $t3,$t2,100		# $t2 = $t2 * %50
	mfhi $t4		# this is remainder from division
	move $t8, $ra
	bnez $t4, onceIncre	# if remainder is exist, result will increase once
	move $v0,$t3
	jr $t8
onceIncre:
	addi $t3,$t3,1		# this result is first rule's result
	move $v0,$t3
	jr $ra


compareResult: 
	# $s4 = first rule's result
	# $s5 = second rule's result
	# $s6 = third rule's result
	
	slt $t1,$s4,$s5		# if($s4 < $s5) $t1 = 1
	bnez $t1,compare2
	slt $t1,$s5,$s6		# if($s5 < $s6) $t1 = 1
	bnez $t1,printRule2
	j printRule3
compare2:
	slt $t1,$s4,$s6		# if($s4 < $s6) $t1 = 1
	bnez $t1,printRule1
	j printRule3

jumpMain:
	jr $ra
printRule1:
	sub $t7, $s4, $t9	# $t7 = result - Ultimate Freedom Fee
	li $v0, 4
	la $a0, giveJurryFee	# "Give JURY FEE to"
	syscall
	li $v0, 1		
	add $a0,$zero,$s7	# "'juryNumber'"
	syscall
	li $v0, 4
	la $a0, print1		# "people and ULTIMATE FREEDOM FEE to the judge. (Rule 1)\n"
	syscall
	la $a0, printSpend	# "You have to spend "
	syscall
	li $v0, 1
	add $a0,$zero,$t9	# " 'ultimate freedom fee' "
	syscall
	li $v0, 4
	la $a0, printSumSign	# " + "
	syscall 
	li $v0, 1
	add $a0,$zero,$t7	# " 'result - ultimate freedom fee' "
	syscall
	li $v0, 4
	la $a0, printEqualSign	# " = "
	syscall
	li $v0, 1
	add $a0,$zero,$s4	# " 'result' "
	syscall
	li $v0, 4
	la $a0, printSpend2 	# "NL at least.\n"
	syscall
	
	j main
printRule2:
	sub $t7,$s5,$t5		# $t7 = result - Freedom with Jury Fee 
	li $v0, 4
	la $a0, giveJurryFee	# "Give JURY FEE to"
	syscall
	li $v0, 1
	add $a0,$zero,$s7	# "'juryNumber'"
	syscall
	li $v0, 4
	la $a0, print2		# "people and FREEDOM WITH JURY FEE to the judge. (Rule 2)\n"
	syscall
	la $a0, printSpend	# "You have to spend "
	syscall
	li $v0, 1
	lw $a0, 0($t5)		# " 'freedom with jury fee' "
	syscall
	li $v0, 4
	la $a0, printSumSign	# " + "
	syscall 
	li $v0, 1
	add $a0,$zero,$t7	# " 'result - Freedom with Jury Fee' "
	syscall
	li $v0, 4
	la $a0, printEqualSign	# " = "
	syscall
	li $v0, 1
	add $a0,$zero,$s5	# " 'result' "
	syscall
	li $v0, 4
	la $a0, printSpend2 	# "NL at least.\n"
	syscall
	j main	
printRule3:
	li $v0, 4
	la $a0, giveJurryFee	# "Give JURY FEE to"
	syscall
	li $v0, 1
	add $a0,$zero,$s7	# "'juryNumber'"
	syscall
	li $v0, 4
	la $a0, print3		# "people and no fee to the judge. (Rule 3)\n"
	syscall
	la $a0, printSpend	# "You have to spend "
	syscall
	li $v0, 1
	lw $a0, juryFee		# "Each Jurry Freedom Fee"
	syscall
	li $v0, 4
	la $a0, printMultSign 	# " * "
	syscall 
	li $v0, 1
	add $a0,$zero,$s7	# "'juryNumber'"
	syscall
	li $v0, 4
	la $a0, printEqualSign	# " = "
	syscall
	li $v0, 1	
	add $a0,$zero,$s6	# " 'result' "
	syscall
	li $v0, 4
	la $a0, printSpend2 	# "NL at least.\n"
	syscall
	j main