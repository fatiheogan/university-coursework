# name: Halil Ibrahim Oymaci
# no:	121044019
# date:	30.11.2015
# info:	partition a given dictionary to the two equal part

legacyItems = {
	'araba':3000,
	'gecekondu':7000,
	'motosiklet':2000,
	'ev malzemeleri':8000,
	'kisisel esyalar':2000,
    'AT':2000,
    'DEVE':1999}

# Find two equal part in a list
def findTwoPart(dictionary):
	values = dictionary.values()
	keys = dictionary.keys()
	sum = 0
	first = []
	second = []
	for price in values:
		sum += price
	part = sum / 2
	i = 0
	done = False
	#find first part
	while( i < len(values) and not done):
		first = []
		first.append(i)
		temp = values[i]
		j = i+1
		while( j < len(values) ):
			temp += values[j]
			if( temp == part ):
				first.append(j)
				done = True
			elif( temp < part ):
				first.append(j)
			else:
				temp -= values[j]
			j += 1
		i += 1
	# find second part
	i = 0
	while( i < len(values) ):
		second.append(i)
		i += 1
	i = 0
	while( i < len(first) ):
		second.remove(first[i])
		i += 1
	# print first part
	print '--first part--'
	i = 0
	while( i < len(first) ):
		print keys[first[i]] + ': ' + str(values[first[i]])
		i += 1
	print 'total: ' + str(part)
	print '--second part--'
	i = 0
	while( i < len(second) ):
		print keys[second[i]] + ': ' + str(values[second[i]])
		i += 1
	print 'total: ' + str(sum - part)

findTwoPart(legacyItems)


#### ACIKLAMA ####
# Yazdigim algoritma ilk elemani seciyor sonra diger elemanlarin degerini bu eleman ile toplayarak artiriyor, her adimda
# listenin toplam degerinin yarisi ile karsilastirma yapiliyor, eger ara toplam kucuk ise eklemeye devam ediliyor,
# eger ara toplam buyukse en son eklenen deger silinip yeni deger ekleniyor, eger ara toplam sonucu esit ise islem bitiyor
# Boylelikle ilk part bulunduktan sonra geriye kalan indisler ikinci part olarak ayarlaniyor.
## EFFIENCY OF ALGORITHM ##
# Algoritma karmasiligini hesaplarsak worst case da calisma durumu:
# ilk adimda n-1 kez icteki loop calisiyor daha sonra n-2 kez calisiyor boyle devam ediyor
# karmasilik = n-1 + n-2 + n-3 + ... + 2 + 1 olur
# Bu sonucta toplam formulunden ((n-1)*n)/2 olur. Bu da elemanidir O(n^2) ( big O (n uzeri iki ))

