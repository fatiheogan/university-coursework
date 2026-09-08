class Specs:
    # constructor
    def __init__(self, weight, value):
        self.weight = weight
        self.value = value
        self.cost = value // weight

    # larger than
    def __lt__(self, other):
        return self.cost < other.cost


def FractionalKnapsack(weight, price, capacity):

    packs = []
    size = len(weight)
    for i in range(size):
        packs.append(Specs(weight[i], price[i]))

    packs.sort(reverse=True)

    result = 0

    for i in packs:
        currentWeight = i.weight
        currentPrice = i.value

        if (capacity - currentWeight >= 0):
            capacity = capacity - currentWeight
            result = result + currentPrice
        else:
            fraction = capacity / currentWeight
            result = result + (currentPrice * fraction)
            capacity = capacity - (currentWeight * fraction)
            break
    return result


def main():
    weight = [30, 20, 20, 50]
    price = [120, 60, 20, 100]
    capacity = 50

    print("Maximum price: ", FractionalKnapsack(weight, price, capacity))


main()
