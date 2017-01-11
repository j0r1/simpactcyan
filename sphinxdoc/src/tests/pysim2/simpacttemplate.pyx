from libcpp.string cimport string
from libcpp cimport bool as cbool
from cython.operator cimport dereference as deref
from cpython.version cimport PY_MAJOR_VERSION

cdef B(s):
    if PY_MAJOR_VERSION < 3:
        return s
    return bytes(s, 'UTF-8')

cdef S(b):
    if PY_MAJOR_VERSION < 3:
        return b
    return b.decode(encoding='UTF-8')

class SimpactException(Exception):
    pass

cdef extern from "../../lib/core/populationinterfaces.h":
    cdef cppclass PopulationAlgorithmInterface:
        PopulationAlgorithmInterface()

    cdef cppclass PopulationStateInterface:
        PopulationStateInterface()

cdef extern from "simpactbindings.h":
    cdef cppclass PersonCXX:
        PersonCXX(double dateOfBirth, cbool isMan)
        void setPythonObject(object o)
        object getPythonObject()
        string getName()
        cbool isMan()
        cbool isWoman()
        double getDateOfBirth()
        double getAgeAt(double t)
        cbool hasDied()
        double getTimeOfDeath()

    cdef cppclass SimpactEventCXX:
        SimpactEventCXX()
        SimpactEventCXX(PersonCXX *p)
        SimpactEventCXX(PersonCXX *p1, PersonCXX *p2)

        void setPythonObject(object o)
        object getPythonObject()

        int getNumberOfPersons()
        PersonCXX *getPerson(int idx)

    cdef cppclass SimpactPopulationCXX:
        SimpactPopulationCXX(PopulationAlgorithmInterface *pAlg, PopulationStateInterface *pState, object pObj)

        bool_t run(double *simTime, long long *maxEvents)

        PersonCXX **getAllPeople()
        PersonCXX **getMen()
        PersonCXX **getWomen()
        PersonCXX **getDeceasedPeople()

        int getNumberOfPeople()
        int getNumberOfMen()
        int getNumberOfWomen()
        int getNumberOfDeceasedPeople()

        void addNewPerson(PersonCXX *pPerson)
        void setPersonDied(PersonCXX *pPerson)
        void markAffectedPerson(PersonCXX *pPerson)

        void onNewEvent(SimpactEventCXX *pEvt)

        double getTime()
        GslRandomNumberGenerator *getRandomNumberGenerator()

cdef extern from "../../lib/mnrm/booltype.h":
    cdef cppclass bool_t:
        bool_t()
        bool_t(cbool value)
        bool_t(string errStr)
        cbool success()
        string getErrorString()

cdef extern from "../../lib/mnrm/gslrandomnumbergenerator.h":
    cdef cppclass GslRandomNumberGenerator:
        GslRandomNumberGenerator()
        GslRandomNumberGenerator(int seed)
        double pickRandomDouble()
        double pickWeibull(double lambd, double kappa)
        double pickWeibull(double lambd, double kappa, double ageMin)
        unsigned long getSeed()

cdef extern from "../../lib/core/populationutil.h":

    cdef cppclass PopulationUtil:
        @staticmethod
        bool_t selectAlgorithmAndState(string alg, GslRandomNumberGenerator &rng, cbool parallel, 
                PopulationAlgorithmInterface **ppAlgo, PopulationStateInterface **ppState)

cdef class GslRng:
    cdef GslRandomNumberGenerator *m_pRngGen
    cdef cbool m_delete;

    def __init__(self, cbool allocate = True, int seed = -1):
        if allocate:
            if seed > 0:
                self.m_pRngGen = new GslRandomNumberGenerator(seed)
            else:
                self.m_pRngGen = new GslRandomNumberGenerator()
            self.m_delete = True
        else:
            self.m_pRngGen = NULL
            self.m_delete = False

    def __dealloc__(self):
        if self.m_delete:
            del self.m_pRngGen

    def getSeed(self):
        if self.m_pRngGen:
            return self.m_pRngGen.getSeed()
        return None

    cdef setRng(self, GslRandomNumberGenerator *pRng):
        if self.m_delete:
            del self.m_pRngGen

        self.m_delete = False
        self.m_pRngGen = pRng

    def pickRandomDouble(self):
        if self.m_pRngGen:
            return self.m_pRngGen.pickRandomDouble()
        return None

    def pickWeibull(self, double lambd, double kappa, ageMin = None):
        if self.m_pRngGen:
            if ageMin is None:
                return self.m_pRngGen.pickWeibull(lambd, kappa)
            else:
                return self.m_pRngGen.pickWeibull(lambd, kappa, ageMin)
        return None

cdef class _Person:
    cdef PersonCXX *m_pPerson
    cdef cbool m_addedToPopulation

    def __init__(self, double dateOfBirth, cbool isMan):
        self.m_pPerson = new PersonCXX(dateOfBirth, isMan)
        self.m_addedToPopulation = False
#        print "Created _Person"

    def __dealloc__(self):
        if not self.m_addedToPopulation:
#            print "Dealloc _Person"
            del self.m_pPerson

    def getName(self):
        return S(self.m_pPerson.getName())

    def isMan(self):
        return self.m_pPerson.isMan()

    def isWoman(self):
        return self.m_pPerson.isWoman()

    def getDateOfBirth(self):
        return self.m_pPerson.getDateOfBirth()

    def getAgeAt(self, double t):
        return self.m_pPerson.getAgeAt(t)	

    def hasDied(self):
        return self.m_pPerson.hasDied()

    def getTimeOfDeath(self):
        return self.getTimeOfDeath()

class Person(_Person):
    def __init__(self, dateOfBirth, isMan):
        super(Person, self).__init__(dateOfBirth, isMan)

cdef class _SimpactEvent:
    cdef SimpactEventCXX *m_pEvt
    cdef cbool m_addedToPopulation

    def __init__(self, _Person p1 = None, _Person p2 = None):
        if p2 and not p1:
            raise SimpactException("Person 2 is set but person 1 isn't")
        if not p1:
            self.m_pEvt = new SimpactEventCXX()
        else:
            if not p2:
                self.m_pEvt = new SimpactEventCXX(p1.m_pPerson)
            else:
                self.m_pEvt = new SimpactEventCXX(p1.m_pPerson, p2.m_pPerson)

        self.m_addedToPopulation = False

    def __dealloc__(self):
        if not self.m_addedToPopulation:
            del self.m_pEvt

    def getPerson(self, int idx):
        if idx < 0 or idx >= self.m_pEvt.getNumberOfPersons():
            return None
        return self.m_pEvt.getPerson(idx).getPythonObject()

class SimpactEvent(_SimpactEvent):
    def __init__(self, p1 = None, p2 = None):
        super(SimpactEvent, self).__init__(p1, p2)

cdef class SimpactPopulation:
    cdef GslRandomNumberGenerator *m_pRng
    cdef SimpactPopulationCXX *m_pPop

    def __init__(self, simType, seed = None):
        cdef PopulationStateInterface *pState
        cdef PopulationAlgorithmInterface *pAlg
        cdef bool_t r
        cdef int rngSeed = -1

        if seed is not None:
            rngSeed = seed

        simTypeStr = B(simType)

        if seed is None:
            self.m_pRng = new GslRandomNumberGenerator()
        else:
            self.m_pRng = new GslRandomNumberGenerator(rngSeed)

        r = PopulationUtil.selectAlgorithmAndState(simTypeStr, deref(self.m_pRng), False, &pAlg, &pState)
        if not r.success():
            raise SimpactException("Unable to get simulation algorithm for type '" + simType + "': " + S(r.getErrorString())) 

        self.m_pPop = new SimpactPopulationCXX(pAlg, pState, self)

    def __dealloc__(self):
        del self.m_pPop

    def getRandomNumberGenerator(self):
        r = GslRng(False)
        r.setRng(self.m_pPop.getRandomNumberGenerator())
        return r

    def run(self, double simTime, long long maxEvents = -1):
        cdef double t
        cdef long long maxEvt
        cdef bool_t r

        if simTime <= 0:
            raise SimpactException("Simulation time must be positive")

        t = simTime
        maxEvt = maxEvents

        r = self.m_pPop.run(&t, &maxEvt)
        if not r.success():
            raise SimpactException("Error running simulation for specified time: " + S(r.getErrorString()))

        return (t, maxEvt)

    def onNewEvent(self, _SimpactEvent evt):
        if not evt:
            raise SimpactException("No event specified")
        if evt.m_addedToPopulation:
            raise SimpactException("Event is already added to a population")

        e = evt.m_pEvt
        e.setPythonObject(evt)
        self.m_pPop.onNewEvent(e)
        evt.m_addedToPopulation = True

    def addNewPerson(self, _Person person):
        if not person:
            raise SimpactException("No person specified")
        if person.m_addedToPopulation:
            raise SimpactException("Person is already added to a population")
        
        p = person.m_pPerson
        self.m_pPop.addNewPerson(p)
        person.m_addedToPopulation = True
        p.setPythonObject(person)

    def setPersonDied(self, _Person person):
        if not person:
            raise SimpactException("No person specified")

        p = person.m_pPerson
        self.m_pPop.setPersonDied(p)

    def markAffectedPerson(self, _Person person):
        if not person:
            raise SimpactException("No person specified")
    
        p = person.m_pPerson
        self.m_pPop.markAffectedPerson(p)

    def getTime(self):
        return self.m_pPop.getTime()

    def getNumberOfPeople(self):
        return self.m_pPop.getNumberOfPeople()

    def getNumberOfMen(self):
        return self.m_pPop.getNumberOfMen()

    def getNumberOfWomen(self):
        return self.m_pPop.getNumberOfWomen()

    def getNumberOfDeceasedPeople(self):
        return self.m_pPop.getNumberOfDeceasedPeople()

    def getPerson(self, idx):
        if idx < 0 or idx >= self.m_pPop.getNumberOfPeople():
            raise SimpactException("Invalid index")
        return self.m_pPop.getAllPeople()[idx].getPythonObject()

    def getMan(self, idx):
        if idx < 0 or idx >= self.m_pPop.getNumberOfMen():
            raise SimpactException("Invalid index")
        return self.m_pPop.getMen()[idx].getPythonObject()

    def getWoman(self, idx):
        if idx < 0 or idx >= self.m_pPop.getNumberOfWomen():
            raise SimpactException("Invalid index")
        return self.m_pPop.getWomen()[idx].getPythonObject()

    def getDeceasedPerson(self, idx):
        if idx < 0 or idx >= self.m_pPop.getNumberOfDeceasedPeople():
            raise SimpactException("Invalid index")
        return self.m_pPop.getDeceasedPeople()[idx].getPythonObject()

    def getPeople(self):
        num = self.getNumberOfPeople()
        for i in range(num):
            yield self.getPerson(i)

    def getMen(self):
        num = self.getNumberOfMen()
        for i in range(num):
            yield self.getMan(i)

    def getWomen(self):
        num = self.getNumberOfWomen()
        for i in range(num):
            yield self.getWoman(i)

    def getDeceasedPeople(self):
        num = self.getNumberOfDeceasedPeople()
        for i in range(num):
            yield self.getDeceasedPerson(i)

# From http://stackoverflow.com/questions/10126668/can-i-override-a-c-virtual-function-within-python-with-cython
cdef public api string cy_call_string_double_func(object self, char* method, double param):
    try:
        func = getattr(self, S(method))
        return B(func(param))
    except Exception as e:
        return B("Warning: exception in " + S(method) + ": " + str(e))

cdef public api void cy_call_void_event_func(object self, char *method, SimpactEventCXX *pEvt):
    try:
        func = getattr(self, S(method))
        e = pEvt.getPythonObject()
        func(e)
    except Exception as e:
        print("Warning: exception in " + S(method) + ": " + str(e))

cdef public api cbool cy_call_bool_func(object self, char *method):
    try:
        func = getattr(self, S(method))
        r = func()
        if not isinstance(r, bool):
            raise SimpactException("Return type is not a boolean")
        return r
    except Exception as e:
        print("Warning: exception in " + S(method) + ": " + str(e))
        return False

cdef public api void cy_call_void_object_func(object self, char *method, object population):
    try:
        func = getattr(self, S(method))
        func(population)
    except Exception as e:
        print("Warning: exception in " + S(method) + ": " + str(e))

cdef public api double cy_call_double_rng_object(object self, char *method, GslRandomNumberGenerator *pRng, object population, double errValue):
    try:
        p = GslRng(False)
        p.setRng(pRng)
        func = getattr(self, S(method))
        return func(p, population)
    except Exception as e:
        print("Warning: exception in " + S(method) + ": " + str(e))
        return errValue

cdef public api double cy_call_double_object_double_double(object self, char *method, object population, double x, double y, double errValue):
    try:
        func = getattr(self, S(method))
        return func(population, x, y)
    except Exception as e:
        print("Warning: exception in " + S(method) + ": " + str(e))
        return errValue

cdef public api cbool cy_call_bool_object_double_func(object self, char *method, object population, double t, string *pErrStr):
    try:
        func = getattr(self, S(method))
        func(population, t)
        return True
    except Exception as e:
        pErrStr[0] = B(str(e))
        return False
