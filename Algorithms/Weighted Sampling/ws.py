import random


class random_sampling:
    import random
    def __init__(self,s) -> None:
        self.count = 0
        self.data = []
        self.limit = s
    def update(self,x):
        self.count += 1
        if len(self.data) < self.limit:
            self.data.append(x)
        else:
            i = random.randrange(1,self.count)
            if i < self.limit:
                self.data[i] = x

    def query(self):
        print(self.data)
        

class weighted_sampling:
    import random
    def __init__(self,s) -> None:
        self.C = []
        self.H = []
        self.tal = 0
        self.limit = s

    def update(self,x):
        if len(self.H) < self.limit:
            self.H.append(x)
            self.H = sorted(self.H, key=lambda tup: tup[1])
        else:
            pass

    def query(self):
        print(self.data)
        #return self.data


array = [(i,i) for i in range(10)]
s = 5
sketch = weighted_sampling(s)
for x in array:
    sketch.update(x)
    sketch.query()
