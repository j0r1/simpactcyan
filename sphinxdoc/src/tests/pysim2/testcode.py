#!/usr/bin/env python
import math
import simpactdebug as simpact
#import simpact


class EventTest(simpact.SimpactEvent):
    def __init__(self, person):
        super(EventTest, self).__init__(person)

    def fire(self, simpactPopulation, t):
        pass

    def getNewInternalTimeDifference(self, rndGen, simpactPopulation):
        r = rndGen.pickRandomDouble();
        dT = -math.log(r)
        return dT

    def calculateInternalTimeInterval(self, algAndState, t0, dt):
        return dt
    
    def solveForRealTimeInterval(self, algAndState, Tdiff, t0):
        return Tdiff

    def getDescription(self, t):
        return "Some description at time " + str(t)

    def isEveryoneAffected(self):
        return False

    def markOtherAffectedPeople(self, pop):
        #print(pop)
        pass

class Man(simpact.Person):
    def __init__(self, dateOfBirth):
        super(Man, self).__init__(dateOfBirth, True)

class Woman(simpact.Person):
    def __init__(self, dateOfBirth):
        super(Woman, self).__init__(dateOfBirth, False)

class TestPopulation(simpact.SimpactPopulation):
    def __init__(self, simType, numMen, numWomen):
        super(TestPopulation, self).__init__(simType)

        for i in range(numMen):
            m = Man(-10)
            m.idx = i
            self.addNewPerson(m)

        for i in range(numWomen):
            w = Woman(-10)
            w.idx = -i
            self.addNewPerson(w)

        numPersons = numMen+numWomen
        for i in range(numPersons):
            p = self.getPerson(i)
            evt = EventTest(p)
            self.onNewEvent(evt)

    #def onAboutToFire(self, evt):
    #    print("onAboutToFire" + str(evt))

    def __str__(self):
        return "TestPopulation with {} men and {} women".format(self.getNumberOfMen(), self.getNumberOfWomen())

def main():
    pop = TestPopulation("opt", 10, 9)
    for p in pop.getWomen():
        print(p.getName())

    try:
        r = pop.run(1000)
        print(r)
    except Exception as e:
        print("Exception:", e)
        
    
    print("End time:", pop.getTime())


    rng = pop.getRandomNumberGenerator()
    print(rng.pickRandomDouble())
    del rng

    del pop
    print("Exiting...")

if __name__ == "__main__":
    main()
