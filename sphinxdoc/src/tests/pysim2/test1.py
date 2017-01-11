#!/usr/bin/env python
from __future__ import print_function
import simpact
import math
import sys
import traceback

possibleInfectionOrigins = [ "Partner", "Mother", "Seed" ]

def tryexcept(func):
    def wrapper(*args):
        try:
            return func(*args)
        except Exception as ex:
            _, _, ex_traceback = sys.exc_info()
            if ex_traceback is None:
                ex_traceback = ex.__traceback__
            tb_lines = traceback.format_exception(ex.__class__, ex, ex_traceback)
            print("%s" % ''.join(tb_lines))

    return wrapper

class Relationship:
    def __init__(self, man, woman, formationTime):
        self.man = man
        self.woman = woman
        self.formationTime = formationTime
        if not self.man.isMan() or not self.woman.isWoman():
            raise Exception("Invalid gender in relationship")

class Person(simpact.Person):
    def __init__(self, dateOfBirth, isMan):
        super(Person, self).__init__(dateOfBirth, isMan)

        self.infectionOrigin = None
        self.relationships = [ ]
        self.sexuallyActive = False

    def addRelationship(self, r):
        self.relationships.append(r)

    def removeRelationship(self, p1, p2):
        for idx in range(len(self.relationships)):
            r = self.relationships[idx]
            if r.man is p1 and r.woman is p2:
                del self.relationships[idx]
                return
        
        raise Exception("Couldn't find relationship between {} and {} in list of person {}".format(p1.getName(), p2.getName(), self.getName()))

    def getNumberOfRelationships(self):
        return len(self.relationships)

class Man(Person):
    def __init__(self, dateOfBirth):
        super(Man, self).__init__(dateOfBirth, True)

class Woman(Person):
    def __init__(self, dateOfBirth):
        super(Woman, self).__init__(dateOfBirth, False)

class EventDebut(simpact.SimpactEvent):
    def __init__(self, person):
        super(EventDebut, self).__init__(person)

    def getDescription(self, tNow):
        p = self.getPerson(0)
        return "Debut of " + p.getName()

    def fire(self, pop, t):
        p = self.getPerson(0)
        if p.isSexuallyActive:
            raise Exception("Person is already sexually active")

        p.isSexuallyActive = True
        if p.isMan():
            for w in pop.getWomen():
                if w.isSexuallyActive:
                    pop.onNewEvent(EventFormation(p, w))
        else:
            for m in pop.getMen():
                if m.isSexuallyActive:
                    pop.onNewEvent(EventFormation(m, p))

    def getNewInternalTimeDifference(self, rndGen, pop):
        p = self.getPerson(0)
        tEvt = p.getDateOfBirth() + pop.getDebutAge()
        return tEvt - pop.getTime()

class EventDissolution(simpact.SimpactEvent):

    a0 = math.log(0.5)
    a1 = 0.0
    a2 = 0.0
    a3 = 0.0
    a4 = 0.0
    a5 = 0.0
    Dp = 0.0
    b = 0.0

    def __init__(self, person1, person2, formationTime):
        super(EventDissolution, self).__init__(person1, person2)
        
        self.formationTime = formationTime

    def getDescription(self, tNow):
        p1 = self.getPerson(0)
        p2 = self.getPerson(1)
        return "Dissolution between %s and %s, relationship was formed at %g (%g ago)" % (p1.getName(), p2.getName(), self.formationTime, tNow-self.formationTime)

    def fire(self, pop, t):
        p1 = self.getPerson(0)
        p2 = self.getPerson(1)

        p1.removeRelationship(p1, p2)
        p2.removeRelationship(p1, p2)

        pop.onNewEvent(EventFormation(p1, p2, t))

    def calculateInternalTimeInterval(self, pop, t0, dt):
        p1 = self.getPerson(0)
        p2 = self.getPerson(1)
        return ExponentialHazardToInternalTime(p1, p2, t0, dt, self.formationTime,
                                               self.a0, self.a1, self.a2, self.a3, 
                                               self.a4, self.a5, self.Dp, self.b)

    def solveForRealTimeInterval(self, pop, Tdiff, t0):
        p1 = self.getPerson(0)
        p2 = self.getPerson(1)
        return ExponentialHazardToRealTime(p1, p2, t0, Tdiff, self.formationTime,
                                               self.a0, self.a1, self.a2, self.a3, 
                                               self.a4, self.a5, self.Dp, self.b)

class EventFormation(simpact.SimpactEvent):

    # a0 = math.log(0.1)
    a1 = 0.0
    a2 = 0.0
    a3 = 0.0
    a4 = 0.0
    a5 = 0.0
    Dp = 0.0
    b = 0.0

    def __init__(self, person1, person2, lastDissTime = None):
        super(EventFormation, self).__init__(person1, person2)

        assert person1.isMan()
        assert person2.isWoman()
        self.lastDissTime = lastDissTime

    def getDescription(self, tNow):
        p1 = self.getPerson(0)
        p2 = self.getPerson(1)
        return "Formation between {} and {}".format(p1.getName(), p2.getName())

    def fire(self, pop, t):
        p1 = self.getPerson(0)
        p2 = self.getPerson(1)
        
        r = Relationship(p1, p2, t)
        p1.addRelationship(r)
        p2.addRelationship(r)

        pop.onNewEvent(EventDissolution(p1, p2, t))

        if self.lastDissTime >= 0:
            print("New formation between %s and %s after %g years" % (p1.getName(), p2.getName(), t-self.lastDissTime))

    def calculateInternalTimeInterval(self, pop, t0, dt):
        p1 = self.getPerson(0)
        p2 = self.getPerson(1)

        n = float(pop.getInitialPopulationSize())
        a0 = math.log(10.0/(n/2.0))
        tr = self.lastDissTime

        tBi = p1.getDateOfBirth()
        tBj = p2.getDateOfBirth()

        if tr is None:
            debut = pop.getDebutAge()
            t1 = tBi + debut
            t2 = tBj + debut
            tr = max(t1, t2)
        
        return ExponentialHazardToInternalTime(p1, p2, t0, dt, tr, a0, self.a1, self.a2,
                                               self.a3, self.a4, self.a5, self.Dp, self.b)

    def solveForRealTimeInterval(self, pop, Tdiff, t0):
        p1 = self.getPerson(0)
        p2 = self.getPerson(1)

        n = float(pop.getInitialPopulationSize())
        a0 = math.log(10.0/(n/2.0))
        tr = self.lastDissTime

        tBi = p1.getDateOfBirth()
        tBj = p2.getDateOfBirth()

        if tr is None:
            debut = pop.getDebutAge()
            t1 = tBi + debut
            t2 = tBj + debut
            tr = max(t1, t2)
        
        return ExponentialHazardToRealTime(p1, p2, t0, Tdiff, tr, a0, self.a1, self.a2,
                                               self.a3, self.a4, self.a5, self.Dp, self.b)


class EventMortality(simpact.SimpactEvent):
    def __init__(self, person):
        super(EventMortality, self).__init__(person)

    def getDescription(self, tNow):
        p = self.getPerson(0)
        return "Death of %s (current age %g)" % (p.getName(), p.getAgeAt(tNow))

    def markOtherAffectedPeople(self, pop):
        p = self.getPerson(0)
        if p.isMan():
            for r in p.relationships:
                assert r.man is p
                pop.markAffectedPerson(r.woman)
        elif p.isWoman():
            for r in p.relationships:
                assert r.woman is p
                pop.markAffectedPerson(r.man)

    def fire(self, pop, t):
        p = self.getPerson(0)
        if p.isMan():
            m = p
            for r in p.relationships:
                w = r.woman
                w.removeRelationship(m, w)

                print("%g\tDeath based dissolution between %s and %s" %(t, m.getName(), w.getName()))
        else:
            assert p.isWoman()
            w = p
            for r in p.relationships:
                m = r.man
                m.removeRelationship(m, w)

                print("%g\tDeath based dissolution between %s and %s" %(t, m.getName(), w.getName()))

        pop.setPersonDied(p) 

    def getNewInternalTimeDifference(self, rndGen, pop):
        p = self.getPerson(0)
        shape = 4.0
        scale = 70.0
        genderDiff = 5.0

        curTime = pop.getTime()
        ageOffset = p.getAgeAt(curTime)

        genderDiff /= 2.0
        if p.isMan():
            genderDiff = -genderDiff

        scale += genderDiff

        return rndGen.pickWeibull(scale, shape, ageOffset) - ageOffset

def printparams(func):
    def wrapper(*args):
        print(args)
        return func(*args)
    return wrapper

def ExponentialHazardToInternalTime(p1, p2, t0, dt, tr, a0, a1, a2, a3, a4, a5, Dp, b):
    Pi = p1.getNumberOfRelationships()
    Pj = p2.getNumberOfRelationships()
    tBi = p1.getDateOfBirth()
    tBj = p2.getDateOfBirth()
    C = a4 + b
    dT = 0

    if C == 0:
        B = math.exp(a0 + a1*Pi + a2*Pj + a3*abs(Pi-Pj) + a4*(t0 - (tBi + tBj)/2.0) + 
                a5*abs(-tBi+tBj-Dp) - b*tr)
        dT = B*dt;
    else:
        E = math.exp(a0 + a1*Pi + a2*Pj + a3*abs(Pi-Pj) + a4*(t0 - (tBi + tBj)/2.0) +
                a5*abs(-tBi+tBj-Dp) + b*(t0-tr))
    
        dT = (E/C)*(math.exp(C*dt)-1.0)
    return dT

def ExponentialHazardToRealTime(p1, p2, t0, Tdiff, tr, a0, a1, a2, a3, a4, a5, Dp, b):
    Pi = p1.getNumberOfRelationships()
    Pj = p2.getNumberOfRelationships()
    tBi = p1.getDateOfBirth()
    tBj = p2.getDateOfBirth()
    C = a4 + b
    dT = 0

    if C == 0:
        B = math.exp(a0 + a1*Pi + a2*Pj + a3*abs(Pi-Pj) + a4*(t0 - (tBi + tBj)/2.0) +
                    a5*abs(-tBi+tBj-Dp) - b*tr)
        dt = Tdiff/B
    else:
        E = math.exp(a0 + a1*Pi + a2*Pj + a3*abs(Pi-Pj) + a4*(t0 - (tBi + tBj)/2.0) +
                    a5*abs(-tBi+tBj-Dp) + b*(t0-tr))
        dt = (E/C)*(math.exp(C*dt)-1.0)

    return dt

class SimpactPopulation(simpact.SimpactPopulation):
    def __init__(self, simType, numMen, numWomen, seed = None):
        super(SimpactPopulation, self).__init__(simType, seed)

        self.initialPopSize = numMen+numWomen

        for i in range(numMen):
            self.addNewPerson(Man(-30.0))
        for i in range(numWomen):
            self.addNewPerson(Woman(-30.0))

        for p in self.getPeople():
            if p.getAgeAt(0.0) >= self.getDebutAge():
                p.isSexuallyActive = True

        for p in self.getPeople():
            self.onNewEvent(EventMortality(p))

        for w in self.getWomen():
            if w.isSexuallyActive:
                for m in self.getMen():
                    if m.isSexuallyActive:
                        self.onNewEvent(EventFormation(m, w))

        for p in self.getPeople():
            if not p.isSexuallyActive:
                self.onNewEvent(EventDebut(p))

    def getDebutAge(self):
        return 15.0

    def getInitialPopulationSize(self):
        return self.initialPopSize

def main():

    numMen = int(sys.argv[1])
    numWomen = int(sys.argv[2])
    tMax = float(sys.argv[3])
    algo = sys.argv[4]
    seed = None
    if len(sys.argv) > 5:
        seed = int(sys.argv[5])

    pop = SimpactPopulation(algo, numMen, numWomen, seed = seed)
    pop.run(tMax)

if __name__ == "__main__":
    main()
