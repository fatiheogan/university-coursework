# name: Halil Ibrahim Oymaci
# no:	121044019
# date:	30.11.2015
# info:	Find number of occurences substring of a given string which starts first character and finish last character

string = "TURKEYTUNUSIAURUGUAYYY"

def numberOfSubString( string, first, last):
	i = 0
	while(i < len(string)):
		if(string[i]==first):
			j = len(string)-1
			while(j>i):
				if(string[j]==last):
					print string[i:j+1]
				j -= 1
		i += 1		

numberOfSubString(string, 'T','Y')