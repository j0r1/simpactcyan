.. This is just a definition of |br| to be able to force a line break somewhere

.. |br| raw:: html

   <br/>

.. _simdetails:

Simulation details
==================

.. _generalflow:

General flow of a simulation
----------------------------

As one might expect, a population consists of persons which can either be
male or female. Persons can be introduced into the simulation in two ways:

 - During the initialization of the simulation, in which case persons with certain ages
   (drawn from a distribution) are added to the simulation.
 - When the simulation is running, and the birth of a new person occurs.

Once born, a person will become sexually active when a :ref:`debut <debut>` event is triggered. 
If the person is introduced into the population at the start of the simulation, and the age
exceeds the debut age, this event is no longer scheduled. Every person always has a 'normal'
:ref:`mortality event <mortality>` scheduled, which corresponds to a cause of death other than AIDS.

To get the HIV epidemic started, an :ref:`HIV seeding event <hivseeding>` can be scheduled.
When this event is triggered, a number of people in the existing population will be
marked as being HIV-infected. An infected person will go through a number of infection
stages. Until a :ref:`chronic stage event <chronicstage>` is triggered the person is in the
acute HIV infection stage; afterwards he will be in the chronic stage. A specific amount
of time before dying of AIDS, an :ref:`AIDS stage event <aidsstage>` is triggered, marking the 
transition of the chronic HIV stage to the actual AIDS stage. Even closer to the AIDS 
related death, another :ref:`AIDS stage event <aidsstage>` is triggered, after which the person is in the
'final AIDS stage', and will be too ill to e.g. form sexual relationships. When the person 
dies of AIDS, the :ref:`AIDS mortality event <aidsmortality>` is fired. Note that it is always
possible that the person dies from other causes; in that case the 'normal' 
:ref:`mortality event <mortality>` will get triggered sooner.

If two persons of opposite gender are sexually active, a relationship can be formed. If this 
is the case, a :ref:`formation event <formation>` will be triggered. When a relationship between two people
exists, it is possible that conception takes place, in which case a :ref:`conception event <conception>`
will be triggered. If this happens, a while later a :ref:`birth event <birth>` will be fired,
and a new person will be introduced into the population. In case one of the partners in
the relationship is HIV infected, transmission of the virus may occur. If so, a
:ref:`transmission event <transmission>` will fire, and the newly infected person will
go through the different infection stages as described earlier. Of course, it is also
possible that the relationship will cease to exist, in which case a :ref:`dissolution event <dissolution>`
will be fired. Note that in the version at the time of writing, there is no 
mother-to-child-transmission (MTCT).

Starting treatment and dropping out of treatment is managed by another sequence of events.
When a person gets infected, either by :ref:`HIV seeding <hivseeding>` or by :ref:`transmission <transmission>`,
first a :ref:`diagnosis event <diagnosis>` is scheduled. If this is triggered, the person is
considered to feel bad enough to go to a doctor and get diagnosed as being infected with
HIV. If this happens, an :ref:`HIV monitoring event <monitoring>` is scheduled to monitor the
progression of the HIV infection. If the person is both eligible and willing to receive 
antiretroviral therapy, treatment is started; if not, a new monitoring event will be 
scheduled. In case treatment is started, no more monitoring events will be scheduled, but
the person will have a chance to drop out of treatment, in which case a :ref:`dropout event <dropout>`
is triggered. When a person drops out of treatment, a new :ref:`diagnosis event <diagnosis>` 
will be scheduled. The rationale is that when a person drops out, he may do so because
he's feeling better thanks to the treatment. After dropping out, the condition will
worsen again, causing him to go to a doctor, get re-diagnosed and start treatment again.

Initialization of the simulation
--------------------------------

During the initialization of the simulated population, the following steps will take place:

 - Create the initial population: 

    - A number of men (``population.nummen``) and women 
      (``population.numwomen``) are added to the population, of which the age is drawn from 
      an age distribution file (``population.agedistfile``). Depending on the :ref:`debut age <debut>`,
      people may be marked as being 'sexually active'.

    - The initial population size will be remembered for use in e.g. the :ref:`formation hazard <formation>`.
      During the simulation, this size can be :ref:`synchronized <syncpopstats>` using another event.

 - Schedule the initial events:

    - For each person, a 'normal' :ref:`mortality event <mortality>` will be scheduled, and if needed,
      a :ref:`debut event <debut>` will be scheduled.
    - Get the HIV epidemic started at some point, by scheduling an :ref:`HIV seeding event <hivseeding>`.
    - If specified, schedule the next :ref:`simulation intervention <simulationintervention>`. This is a
      general way of changing simulation settings during the simulation.
    - Schedule a :ref:`periodic logging event <periodiclogging>` if requested. This will log some 
      statistics about the simulation at regular intervals.
    - In case the population size is expected to vary much, one can request an event to 
      :ref:`synchronize <syncpopstats>` the remembered population size for use in other events.
    - For pairs of sexually active persons, depending on the :ref:`'eyecap' <eyecap>` settings
      (``population.eyecap.fraction``), schedule :ref:`formation events <formation>`

Once the simulation is started, it will run either until the number of years specified in
``population.simtime`` have passed, or until the number of events specified in 
``population.maxevents`` have been executed.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``population.nummen`` (100): |br|
   The initial number of men when starting the simulation.

 - ``population.numwomen`` (100): |br|
   The initial number of women when starting the simulation.

 - ``population.simtime`` (15): |br|
   The maximum time that will be simulated, specified in years.

 - ``population.maxevents`` (-1): |br|
   If greater than zero, the simulation will stop when this
   number of events has been executed. This is not used if negative.

 - ``population.agedistfile`` ( "sa_2003.csv" in the Simpact Cyan data directory): |br|
   This is
   a CSV file with three columns, named 'Age', 'Percent Male' and 'Percent Female'. The
   values of the age are specified in years and should be increasing;  the specified percentages are deemed valid
   until the next age. The default is the age distribution in South Africa from 2003.  
  
   Note that **when using the R or Python method** to start simulations, you need to
   specify the age distribution as a parameter to the ``run`` function, if you want
   to use any other distribution than the default. See the :ref:`R section <startingfromR>`
   or :ref:`Python section <startingfromPython>` for more information.

.. _eyecap:

 - ``population.eyecap.fraction`` (1): |br|
   This parameter allows you to
   specify with how many persons of the opposite sex (who are sexually active), 
   specified as a fraction, someone can possibly have relationships. If set to the
   default value of one, every man can possibly engage in a relationship with every
   woman (and vice versa) causing :math:`O(N^2)` formation events to be scheduled.
   For larger population sizes this large amount of events will really slow things down,
   and because in that case it is not even realistic that everyone can form a relationship
   with everyone else, a lower number of this 'eyecap fraction' (for which `'blinders' or 'blinkers' <http://en.wikipedia.org/wiki/Blinkers_%28horse_tack%29>`_
   is a better name) will cause a person to be interested in fewer people. 
   
   When each person is assigned the trivial location (0, 0), the people for such a 
   limited set of interests are simply chosen at random. If a non-trivial 2D distribution is used
   (see the section about :ref:`the geographical location <geodist>` of a person), the
   set of these interests will preferably be chosen closer to the location of the
   person in question. To do this, instead of a really accurate ordering of everyone
   based on their distance (which would become very slow for large populations), an
   approximate :ref:`coarse grid <coarsegrid>` is used instead (see below).
   
   In case you want to disable relationship formation altogether, you can set this value to zero.

.. _coarsegrid:
 
 - ``population.coarsemap.subdivx`` (20): |br|
   As described above, in case the :ref:`'eyecap' <eyecap>` setting is used, each person
   will have a set of interests assigned to them. For issues of speed, a coarse grid
   is used to get an approximation of the ordering by distance.

   To do so, a 2D grid is made that covers the region of the persons' locations,
   and each person is assigned to a corresponding grid cell. To get an approximate
   ordering of other people with respect to a certain location, the grid cells themselves
   are ordered and people are selected based on this ordering to create the set of 
   'interests'.

   The value of this parameter describes the number of grid cells in the x-direction.

 - ``population.coarsemap.subdivy`` (20): |br|
   Similar to the previous setting, the value of this parameter describes the number of 
   grid cells in the y-direction.

.. _populationmsm:

 - ``population.msm`` ('no'): |br|
   If ``no`` (the default), only heterosexual relationships will be possible. If set to
   ``yes``, MSM relationships will be possible as well.

.. _person:

Per person options
------------------

As explained before, a population is central to the Simpact Cyan simulation and
such a population consists of persons, each of which can be a man or a woman.
During the simulation, these persons have many properties: gender, age,
the number of relationships, which partners, etc. Several properties of persons
can be defined using configuration options, which are discussed in this section.

.. _viralload:

HIV Viral load related options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Several options are related to the viral load of a person. When a person becomes
HIV-infected, either by an :ref:`HIV seeding event <hivseeding>` or because of
:ref:`transmission <transmission>` of the virus, a set-point viral load value is
chosen and stored for this person. When a person receives treatment, the viral
load is lowered (see the :ref:`monitoring event <monitoring>`) and if the person
drops out of treatment the initially chosen set-point viral load is restored.

.. _viralloadx:

The set-point viral load is the viral load that the person has during the
chronic stage. In the acute stage or in the AIDS stages, the configuration
values ``person.vsp.toacute.x``, ``person.vsp.toaids.x`` and ``person.vsp.tofinalaids.x``
cause the real viral load to differ from the set-point viral load in such a
way that the transmission probability (see the :ref:`transmission event <transmission>`)
is altered: the hazard for transmission will increase by the factor ``x`` that
is defined this way. There is a limit to the new viral load that can arise
like this, which can be controlled using the option ``person.vsp.maxvalue``.

There are currently two models for initializing the set-point viral load and
determining what happens during transmission of the viral load, i.e. to
which degree the set-point viral load of the infector is inherited. The model
type is controlled using the option ``person.vsp.model.type`` which can be
either ``logdist2d`` or ``logweibullwithnoise``. In case it's ``logdist2d``, a two
dimensional probability distribution is used to model the transmission and
initialization of the (base 10 logarithm) set-point viral load values:

.. math::

    {\rm prob}(v_{\rm infector}, v_{\rm infectee})

The precise probability distribution that is used can be controlled using the
``person.vsp.model.logdist2d.dist2d.type`` config setting.
By default, when an :ref:`HIV seeding event <hivseeding>` takes place, the base 10
logarithm of a set-point viral load value is chosen from the marginal distribution:

.. math::

    {\rm prob}(v_{\rm infectee}) = \int {\rm prob}(v_{\rm infector}, v_{\rm infectee}) d v_{\rm infector}

In case another distribution needs to be used, this behavour can
be overridden by setting ``person.vsp.model.logdist2d.usealternativeseeddist`` to
``yes`` and configuring ``person.vsp.model.logdist2d.alternativeseed.dist.type`` to
the desired one dimensional probability distribution (again for the base 10 
logarithm of the set-point viral load).

Upon transmission, the associated conditional probability is used:

.. math::

    {\rm prob}(v_{\rm infectee} | v_{\rm infector})

If the other viral load model (``logweibullwithnoise``) is used, the base 10 logarithm of
the set-point viral load in case of a seeding event, is chosen from from a
`Weibull distribution <http://en.wikipedia.org/wiki/Weibull_distribution>`_ with
parameters specified by ``person.vsp.model.logweibullwithnoise.weibullscale``
and ``person.vsp.model.logweibullwithnoise.weibullshape``. Upon transmission,
the infectee inherits the the set-point viral load value from the infector,
but some randomness is added. The added value is drawn from a normal distribution
of which the mean is zero, and the standard deviation is set to a fraction of
the set-point viral load value of the infector (controlled by
``person.vsp.model.logweibullwithnoise.fracsigma``). In this approach, it is
possible that the new set-point viral load becomes negative, which is not
realistic of course. The value of ``person.vsp.model.logweibullwithnoise.onnegative``
determines what needs to be done in this case: if it's ``logweibull``, a new
set-point viral load value will be chosen from the Weibull distribution, in the
same way as what happens during HIV seeding. In case it's ``noiseagain``, a new
noise value is added to the infector's set-point viral load.

.. _cd4count:

When a person becomes HIV-infected, the simulation already fixes the CD4 values
at the time of infection (controlled by ``person.cd4.start.dist.type``) and
at the time of AIDS related death (controlled by ``person.cd4.end.dist.type``).
At any other point in time, the CD4 count of that person will simply be a
linear interpolation between the initial value and the value at time of death.
Note that the time of AIDS-related death can vary due to treatment or
dropping out.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``person.vsp.toacute.x`` (10.0): |br|
   The set-point viral load of a person is that person's reference value. When
   the viral load during the acute stage is needed, it is determined in such a
   way that the :ref:`transmission hazard <transmission>` increases by this factor,
   possibly clipped to a maximum value (see ``person.vsp.maxvalue``).

 - ``person.vsp.toaids.x`` (7.0): |br|
   The set-point viral load of a person is that person's reference value. When
   the viral load during the initial AIDS stage is needed, it is determined in such a
   way that the :ref:`transmission hazard <transmission>` increases by this factor,
   possibly clipped to a maximum value (see ``person.vsp.maxvalue``).

 - ``person.vsp.tofinalaids.x`` (12.0): |br|
   The set-point viral load of a person is that person's reference value. When
   the viral load during the final AIDS stage is needed, it is determined in such a
   way that the :ref:`transmission hazard <transmission>` increases by this factor,
   possibly clipped to a maximum value (see ``person.vsp.maxvalue``).

 - ``person.vsp.maxvalue`` (1e9): |br|
   When determining the viral load during acute, AIDS or final AIDS stages (see
   previous options), a check is done so that the value does not exceed this maximum. 
   If necessary, the calculated viral load value is clipped to this maximum value.

 - ``person.vsp.model.type`` ('logdist2d'): |br|
   When initializing the set-point viral load value during an :ref:`HIV seeding event <hivseeding>`
   or due to :ref:`transmission <transmission>` of the virus, one of two methods will
   be used. As explained above, valid options here are ``logdist2d`` and ``logweibullwithnoise``.

 - ``person.vsp.model.logdist2d.dist2d.type`` ('binormalsymm' between 1 and 8, with mean 4, sigma 1 and correlation 0.33): |br|
   This is only used if the model type is set to ``logdist2d``. It specifies the two
   dimensional distribution that should be used for HIV transmission, and that can be used
   for initialization during an :ref:`HIV seeding event <hivseeding>`. The distribution
   is used to pick set-point viral load values on a base 10 logarithmic scale. As
   :ref:`explained before <probdists>`, other :ref:`two dimensional distribution <prob2d>`
   than the default can be used as well.

 - ``person.vsp.model.logdist2d.usealternativeseeddist`` ('no'): |br|
   In the ``logdist2d`` model, by default the marginal distribution is used to initialize
   set-point viral load values when HIV seeding is triggered. If a different one dimensional
   distribution should be used for this, this option needs to be set to ``yes``, and the
   appropriate distribution should be configured in ``person.vsp.model.logdist2d.alternativeseed.dist.type``.

 - ``person.vsp.model.logdist2d.alternativeseed.dist.type`` ('fixed'): |br|
   In case the previous option is set to yes, you need to set this to a valid distribution.
   The default ``fixed`` distribution with a value of 0 is *not* a good choice here.

 - ``person.vsp.model.logweibullwithnoise.weibullscale`` (5.05): |br|
   In case ``person.vsp.model.type`` is set to ``logweibullwithnoise``, this controls
   the scale parameter that is used for the Weibull distribution to initialize
   set-point viral load values (on a base 10 logarithmic scale).

 - ``person.vsp.model.logweibullwithnoise.weibullshape`` (7.2): |br|
   In case ``person.vsp.model.type`` is set to ``logweibullwithnoise``, this controls
   the shape parameter that is used for the Weibull distribution to initialize
   set-point viral load values (on a base 10 logarithmic scale).

 - ``person.vsp.model.logweibullwithnoise.fracsigma`` (0.1): |br|
   In case ``person.vsp.model.type`` is set to ``logweibullwithnoise``, upon transmission
   of the virus, the set-point viral load is inherited but some noise is added.
   As explained earlier, this specifies the relative size of the noise.

 - ``person.vsp.model.logweibullwithnoise.onnegative`` ('logweibull'): |br|
   After adding noise in the ``logweibullwithnoise`` model, it is possible that the
   new set-point viral load is a negative value, which is invalid. This parameter
   specifies what needs to be done in this case. If it's ``logweibull``, then the
   Weibull distribution is used to choose a new set-point viral load again. If
   the setting is ``noiseagain``, a new noise value will be used, until the new
   set-point viral load is a valid number.

 - ``person.cd4.start.dist.type`` ('uniform' between 700 and 1300): |br|
   Specifies the :ref:`one dimensional distribution <prob1d>` which is used to draw the
   initial CD4 value from. This is the CD4 value the person has at the time of
   infection.

 - ``person.cd4.end.dist.type`` ('uniform' between 0 and 100): |br|
   Specifies the :ref:`one dimensional distribution <prob1d>` which is used to draw the
   final CD4 value from. This is the CD4 value the person will have when he dies from
   AIDS related causes.

.. _personhsv2opts:

HIV and HSV2 related settings
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For the :ref:`HIV transmission <transmission>` hazard, person-dependent values for susceptibility for both infections and susceptibility for HIV only can be set. These values will be drawn from the distributions specified by ``person.hiv.b0.dist.type`` and ``person.hiv.b1.dist.type`` respectively, and their corresponding parameters.

For the :ref:`HSV2 transmission <hsv2transmission>` hazard, a person-dependent baseline
value can be set. This value will be drawn from the distribution specified
by ``person.hsv2.a.dist.type`` and its corresponding parameters. Furthermore, a person-dependent value for susceptibility for HSV2 only can be set. This value will be drawn from a distribution specified by ``person.hsv2.b2.dist.type`` and its corresponding parameters. Furthermore, the value for susceptibility for both infections, drawn from the distribution specified by ``person.hiv.b0.dist.type`` will also be included in the :ref:`HSV2 transmission <hsv2transmission>` hazard.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``person.hiv.b0.dist.type`` ('fixed' with value 0): |br|
   Specifies the :ref:`one dimensional distribution <prob1d>` that is used to draw the person dependent value for susceptibility for both infections from the :ref:`HIV transmission <transmission>` and the :ref:`HSV2 transmission <hsv2transmission>` hazards.
 - ``person.hiv.b1.dist.type`` ('fixed' with value 0): |br|
   Specifies the :ref:`one dimensional distribution <prob1d>` that is used to draw the person dependent value for susceptibility for HIV only from the :ref:`HIV transmission <transmission>` hazard.
 - ``person.hsv2.a.dist.type`` ('fixed' with value 0): |br|
   Specifies the :ref:`one dimensional distribution <prob1d>` that is used to draw
   the person dependent baseline value from for the :ref:`HSV2 transmission <hsv2transmission>`
   hazard.
 - ``person.hsv2.b2.dist.type`` ('fixed' with value 0): |br|
   Specifies the :ref:`one dimensional distribution <prob1d>` that is used to draw the person dependent value for susceptibility for HSV2 only for the :ref:`HSV2 transmission <hsv2transmission>` hazard.

If you want to include the influence of susceptibility in your simulations, appropriate distributions for :math:`b_{\rm 0}`, :math:`b_{\rm 1}` and :math:`b_{\rm 2}` are normal distributions, so that the expected values of :math:`\exp(b_{\rm 0}+b_{\rm 1})` and :math:`\exp(b_{\rm 0}+b_{\rm 2})` are both equal to 1. For :math:`b_{\rm i} \sim\ N\left(\mu_{i},\sigma^{2}_{i}\right)`, normal probability distributions with :math:`\mu_{i}= - \sigma^{2}_{i}/2` (:math:`i=0,1,2`) fulfill these condions. For more information, see `probability_distributions.pdf <_static/probability_distributions.pdf>`_.

Relationship related settings
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _eagerness:

In some hazards, the eagerness of a person to form a relationship is used, to
allow for per-person variation. Such an eagerness value can be defined for
heterosexual relationships (``person.eagerness.man.dist.type`` and
``person.eagerness.woman.dist.type``) and homosexual relationships
(``person.eagerness.man.msm.dist.type`` and ``person.eagerness.woman.wsw.dist.type``)
independently, or a correlation can be introduced by using a joint distribution
(``person.eagerness.man.joint.dist2d`` and ``person.eagerness.woman.joint.dist2d``).
In the latter case, a pair of numbers is generated from a distribution,
of which the first number is interpreted as the eagerness for a heterosexual
relationship and the second for a homosexual relationship. By default, independent
random numbers are used, but this can be changed using the configuration
value of ``person.eagerness.man.type`` and ``person.eagerness.woman.type``.

.. _personagegap:

Similarly, preferred age gaps can be used in hazards, and these also can be
defined for each person separately. Moreover, they can differ between heterosexual
relationships (``person.agegap.man.dist.type`` and ``person.agegap.woman.dist.type``)
and homosexual ones (``person.agegap.man.msm.dist`` and ``person.agegap.woman.wsw.dist``).

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``person.eagerness.man.type`` ('independent'): |br|
   This can be set to ``independent`` or ``joint``, and specifies if the eagerness
   values for heterosexual and homosexual relationship formation for men should be chosen
   independently or from a joint distribution.

 - ``person.eagerness.man.dist.type`` ('fixed' with value 0): |br|
   Specifies the :ref:`one dimensional distribution <prob1d>` the eagerness parameter for
   a man and for heterosexual relationships is chosen from. Is only used if
   ``person.eagerness.man.type`` is set to ``independent``.

 - ``person.eagerness.man.msm.dist.type`` ('fixed' with value 0): |br|
   Specifies the :ref:`one dimensional distribution <prob1d>` the eagerness parameter for
   a man and for homosexual relationships is chosen from. Is only used if
   ``person.eagerness.man.type`` is set to ``independent``.

 - ``person.eagerness.man.joint.dist2d`` ('fixed' with value (0,0)): |br|
   This is only used if ``person.eagerness.man.type`` is set to ``joint``. In this case
   the eagerness values for a man will be based on a pair of numbers chosen from a
   :ref:`two dimensional distribution <prob2d>`, of which the first number will be
   interpreted as the eagerness for a heterosexual relationship and the second
   one as the eagerness for a homosexual relationship.

 - ``person.eagerness.woman.type`` ('independent'): |br|
   This can be set to ``independent`` or ``joint``, and specifies if the eagerness
   values for heterosexual and homosexual relationship formation for women should be chosen
   independently or from a joint distribution.

 - ``person.eagerness.woman.dist.type`` ('fixed' with value 0): |br|
   Specifies the :ref:`one dimensional distribution <prob1d>` the eagerness parameter for
   a woman and for heterosexual relationships is chosen from. Is only used if
   ``person.eagerness.woman.type`` is set to ``independent``.

 - ``person.eagerness.woman.wsw.dist.type`` ('fixed' with value 0): |br|
   Specifies the :ref:`one dimensional distribution <prob1d>` the eagerness parameter for
   a woman and for homosexual relationships is chosen from. Is only used if
   ``person.eagerness.woman.type`` is set to ``independent``.

 - ``person.eagerness.woman.joint.dist2d`` ('fixed' with value (0,0)): |br|
   This is only used if ``person.eagerness.woman.type`` is set to ``joint``. In this case
   the eagerness values for a woman will be based on a pair of numbers chosen from a
   :ref:`two dimensional distribution <prob2d>`, of which the first number will be
   interpreted as the eagerness for a heterosexual relationship and the second
   one as the eagerness for a homosexual relationship.

 - ``person.agegap.man.dist.type`` ('fixed' with value 0): |br|
   Specifies the :ref:`one dimensional distribution <prob1d>` the preferred age gap for
   a man, for heterosexual relationships, is chosen from.

 - ``person.agegap.man.msm.dist`` ('fixed' with value 0): |br|
   Specifies the :ref:`one dimensional distribution <prob1d>` the preferred age gap for
   a man, for homosexual relationships, is chosen from.

 - ``person.agegap.woman.dist.type`` ('fixed' with value 0): |br|
   Specifies the :ref:`one dimensional distribution <prob1d>` the preferred age gap for
   a woman, for heterosexual relationships, is chosen from.

 - ``person.agegap.woman.wsw.dist`` ('fixed' with value 0): |br|
   Specifies the :ref:`one dimensional distribution <prob1d>` the preferred age gap for
   a woman, for homosexual relationships, is chosen from.

Various other settings
^^^^^^^^^^^^^^^^^^^^^^

.. _artacceptthreshold:

Here, we'll discuss a few per-person settings which do not fall into the categories
above. The first one is called ``person.art.accept.threshold.dist.type`` and is related
to how willing a person is to start treatment when offered. When a person is introduced
into the population, a number is picked from the specified distribution. This number
is fixed for this person, and will no longer change during the simulation. Then, when
the person is offered treatment, a new random number between 0 and 1 is chosen uniformly.
If this number is below the threshold value that was determined earlier, treatment
will be accepted, otherwise it is rejected. By default, the ``person.art.accept.threshold.dist.type``
setting always sets the threshold at 0.5, causing each person to have a 50% chance of
accepting treatment when offered.

.. _geodist:

When a person is added to the population, a location is chosen for this person from
the :ref:`two dimensional distribution <prob2d>` that is specified in ``person.geo.dist2d.type``.
In the default Simpact Cyan simulation, this location is not yet used in any hazard,
and the default location is put to (0, 0) for everyone. Because the location is written
to :ref:`the person log file <personlog>`, it can be (ab)used to test two dimensional
distributions, like we did in the example for the :ref:`discrete two dimensional distribution <prob2ddiscrete>`.

.. _survdist:

By default, the survival time for a person after becoming HIV infected, is given
by a simple relation based on the set-point viral load. Because an exact mapping
from viral load to survival time is not that realistic, you can add some randomness
to this relation using the distribution in `person.survtime.logoffset.dist.type`.
When a person becomes infected, a random number is drawn from this distribution
and will correspond to an offset in the survival time, as explained in the
:ref:`AIDS mortality event <aidsmortality>`. The following IPython notebook illustrates
the effect: `survivaltimernd.ipynb <_static/survivaltimernd.ipynb>`_.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``person.art.accept.threshold.dist.type`` ('fixed' with value 0.5): |br|
   This specifies the :ref:`one dimensional distribution <prob1d>` that is used to
   choose the ART acceptance threshold for each person, as explained earlier.

 - ``person.geo.dist2d.type`` ('fixed' with value (0, 0)): |br|
   This :ref:`two dimensional distribution <prob2d>` is used to assign a geographic
   location to each person. In the main Simpact Cyan simulation, this is currently
   not used in any hazard.

 - ``person.survtime.logoffset.dist.type`` ('fixed' with value 0): |br|
   This :ref:`one dimensional distribution <prob1d>` can be used to add some randomness
   to the :ref:`survival time <aidsmortality>` until dying of AIDS related causes after 
   becoming HIV infected.

.. _events:

Events
------

The simulation advances by figuring out which event should take place next, followed by
executing code for that event. At the start, many initial events
are typically scheduled, some set up to fire at a specific simulation time, some based on
a hazard which may change during the simulation. During the simulation, new events 
will get scheduled, and some already scheduled
events will be discarded (for example, in case someone dies, no other events involving this
person will need to get executed anymore). 

Below you can find an overview of the events that are currently used in the simulation.
The relevant configuration options are mentioned as well.

.. _aidsmortality:

AIDS mortality event
^^^^^^^^^^^^^^^^^^^^

When a person gets infected with HIV, an HIV-based time of death is determined. This time
of death is determined as the time of infection plus the survival time, which is given by
the following formula (based on :ref:`[Arnaout et al.] <ref_arnaout>`):

.. math::

    t_{\rm survival} = \frac{C}{V_{\rm sp}^{-k}} \times 10^{\rm x}


In this formula, :math:`C` and :math:`k` are parameters which can be configured using the settings
``mortality.aids.survtime.C`` and ``mortality.aids.survtime.k`` respectively. The :math:`x` parameter
is :ref:`determined per person <survdist>` allowing some randomness in the formula: it
determines an offset on a logarithmic scale. By default, this value is zero however, 
causing a very strict relationship between :math:`V_{\rm sp}` and :math:`t_{\rm survival}`. The value of
:math:`V_{\rm sp}` is the set-point viral load, :ref:`first determined at the time of infection <viralload>` 
and in general
different per person. The value of the set-point viral load can change when treatment is involved: when a
person is receiving treatment, the viral load will go down, causing him to live longer.
When a person drops out of treatment, the viral load goes up again and the expected
lifespan shrinks.

To illustrate how this is taken into account, consider a person that has an initial
viral load that causes a survival time of 10 years. Suppose that after 1 year, treatment is started and that
using the formula above the survival time would become 50 years. When treatment got
started, 10% of the survival time had already passed and we take this into account.
So after starting treatment, the AIDS related mortality would be scheduled after
45 years. If the person drops out of treatment 10 years later, 20% of the remaining
survival time has passed, which translates to 2 years in terms of the original viral
load. This means that still 7 years will remain until the AIDS based mortality event
is fired. Note that using this approach, one will never encounter the situation where
the time of death has already passed when increasing the viral load.

You can find an IPython notebook that illustrates this example here: 
`aidsmortalityexample.ipynb <_static/aidsmortalityexample.ipynb>`_

An AIDS based mortality event will be scheduled to fire at the specified time, which
may still change as explained above. When it fires, the person is considered to 
have died from AIDS. Note that this does
not influence the 'normal' :ref:`mortality <mortality>` event, which can still get triggered
sooner to allow for another cause of death.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``mortality.aids.survtime.C`` (1325.0): |br|
   The value of :math:`C` in the formula for :math:`t_{\rm survival}`.

 - ``mortality.aids.survtime.k`` (-0.49): |br|
   The value of :math:`k` in the formula for :math:`t_{\rm survival}`.

.. _aidsstage:

AIDS stage event
^^^^^^^^^^^^^^^^

When a person gets infected with HIV, he will first be in the acute phase of infection,
then in the chronic stage, and after a while in the AIDS stage. The AIDS stage is actually
split in two separate phases: an AIDS stage, and a final AIDS stage. In this last period,
the person is deemed to be too ill to e.g. form sexual relationships.

The first AIDS stage gets scheduled when the :ref:`chronic stage event <chronicstage>` fires,
and is scheduled to get triggered at a specific time (`aidsstage.start`) before the 
:ref:`AIDS related death <aidsmortality>` takes place. When this event fires, another one
is scheduled to mark the transition to the final AIDS stage, also set to take place
a specific amount of time (``aidsstage.final``) before the AIDS based death. Because the
time of the AIDS related death can still change when treatment is involved, these fire times
can also still change.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``aidsstage.start`` (1.25): |br|
   The time before the AIDS related death a person will advance
   to the AIDS stage of infection. Defaults to 15 months.
 - ``aidsstage.final`` (0.5): |br|
   The time before the AIDS related death a person will advance
   to the final AIDS stage of infection. Defaults to 6 months.

.. _birth:

Birth event
^^^^^^^^^^^

After a :ref:`conception event <conception>` is triggered, a new birth event will be scheduled,
so that the woman in the relationship will give birth to a new person a specific time
(based on ``birth.pregnancyduration.dist.type``) later. The gender is determined by the 
``birth.boygirlratio`` configuration setting.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``birth.boygirlratio`` (1.0/2.01): |br|
   The probability of the newly born person to be a man.

 - ``birth.pregnancyduration.dist.type`` (defaults to 'fixed' with a value of 268/365): |br|
   With this parameter you can specify the distribution to be used when determining
   how long the pregnacy should be, before firing the birth event. By default, the
   fixed value of 268/365 is used, but :ref:`other distributions <prob1d>` and related parameters
   can be used as well.

Check stopping criterion event
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This event allows you to terminate a simulation if a certain population size
(``checkstop.max.popsize``) or real-world elapsed time (``checkstop.max.runtime``)
is exceeded. To enable this, the ``checkstop.interval`` parameter must be
set to a positive value. If so, at regularly spaced times (simulation time)
this check will be performed.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``checkstop.interval`` (-1): |br|
   To enable this event, set the value to a positive value. If enabled, this
   event will fire at simulation times that are multiples of this interval,
   at which time checks on the population size and/or running time are performed.
 - ``checkstop.max.runtime`` (inf): |br|
   When the event fires, the elapsed real-world time since the start of the
   simulation program will be compared to this value. If it exceeds it, the simulation
   will be aborted. 
 - ``checkstop.max.popsize`` (inf): |br|
   When the event fires, the population size will be compared to this value. If it 
   exceeds it, the simulation will be aborted. 

.. _chronicstage:

Chronic AIDS stage event
^^^^^^^^^^^^^^^^^^^^^^^^

When a person becomes HIV infected, he starts out in the acute stage of the disease.
This 'chronic stage' event is then scheduled to mark the transition from the acute
stage to the chronic stage, which will
fire a specific amount of time (``chronicstage.acutestagetime``) later.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``chronicstage.acutestagetime`` (0.25): |br|
   This is the duration of the acute HIV stage, before transitioning to the chronic
   stage. The default is three months.

.. _conception:

Conception event
^^^^^^^^^^^^^^^^

When a :ref:`formation event <formation>` has fired (so a man and a woman are in a sexual
relationship), a conception event will be scheduled unless the woman is already
pregnant. This is a hazard-based event, and its hazard at time :math:`t` is defined as:

.. math::

    {\rm hazard} = \exp\left(\alpha_{\rm base} 
                 + \alpha_{\rm age,man}\left(t - t_{\rm birth,man}\right)
                 + \alpha_{\rm age,woman}\left(t - t_{\rm birth,woman}\right)
                 + \alpha_{\rm wsf}\times{\rm WSF}
                 + \right(t-t_{\rm ref}\left)\beta
                   \right)

which is a time-dependent hazard of type

.. math::

    {\rm hazard} = \exp(A+Bt)

By default, only the :math:`\alpha_{\rm base}` value is used (``conception.alpha_base``), resulting in a constant
hazard, but other factors can be used as well: the age of the man and woman in the
relationship can be taken into account using ``conception.alpha_ageman`` and 
``conception.alpha_agewoman``, the weekly sex frequency (WSF) using ``conception.alpha_wsf`` and the
'age' of the relationship using ``conception.beta`` (:math:`t_{\rm ref}` is set to the time 
the relationship started).
The value of :math:`{\rm WSF}` itself is currently chosen from the distribution specified
in ``conception.wsf.dist.type``, at the time the event gets scheduled.

When a conception event fires, so when actual conception takes place, a :ref:`birth event <birth>`
will be scheduled.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``conception.alpha_base`` (-3): |br|
   The value of :math:`\alpha_{\rm base}` in the formula for the hazard.
 - ``conception.alpha_ageman`` (0): |br|
   The value of :math:`\alpha_{\rm age,man}` in the formula for the hazard, to be able
   to take the age of the man in the relationship into account.
 - ``conception.alpha_agewoman`` (0): |br|
   The value of :math:`\alpha_{\rm age,woman}` in the formula for the hazard, to be able
   to take the age of the woman in the relationship into account.
 - ``conception.alpha_wsf`` (0): |br|
   The value of :math:`\alpha_{\rm wsf}` in the formula to the hazard. This way you can
   take a value for the weekly sex frequency (WSF) into account.
 - ``conception.beta`` (0): |br|
   The value of :math:`\beta` in the hazard formula, allowing you to influence the hazard
   based on the 'age' of the relationship.
 - ``conception.t_max`` (200): |br|
   As explained in the section about :ref:`'time limited' hazards <timelimited>`, an
   exponential function needs some kind of threshold value (after which it stays
   constant) to be able to perform the necessary calculations. This configuration
   value is a measure of this threshold.
 - ``conception.wsf.dist.type`` ('fixed', with value 0): |br|
   When the conception event is scheduled, a value for the weekly sex frequency (WSF)
   to use in the hazard formula is picked from a :ref:`distribution <prob1d>`. This configuration
   option specifies which distribution you would like to use, and depending on the
   value other parameters for the distribution can be configured as well.

.. _debut:

Debut event
^^^^^^^^^^^

Persons who are not yet sexually active will have a debut event scheduled, which
will fire when a person has reached a specified age (``debut.debutage``). When
this event fires, the person becomes sexually active and :ref:`relationship formation events <formation>`
will get scheduled. The number of formation events that gets scheduled can
be controlled using the :ref:`'eyecap' <eyecap>` setting.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``debut.debutage`` (15): |br|
   The age a person must have to become sexually active. This determines when the
   debut event for a particular person will get executed.

.. _diagnosis:

Diagnosis event
^^^^^^^^^^^^^^^

When a person gets infected with HIV, either by :ref:`transmission <transmission>`
of the virus or by :ref:`seeding <hivseeding>` the population to get the epidemic
started, a diagnosis event will get scheduled. When fired, the person is deemed
to feel bad enough to go to a doctor and get diagnosed as being HIV-infected.
Upon diagnosis, a :ref:`monitoring event <monitoring>` will be scheduled very
shortly afterwards, to monitor the progression of the disease and to offer 
treatment if eligible. 

This event is hazard-based, and the hazard is of the following form:

.. math::

    \begin{eqnarray}
    {\rm hazard} & = & \exp\left({\rm baseline} + {\rm agefactor}\times(t-t_{\rm birth}) + {\rm genderfactor}\times{\rm G}\right. \\
                     &   & + {\rm diagpartnersfactor}\times {\rm P} +{\rm isdiagnosedfactor}\times D +\beta(t-t_{\rm infected})\\
			     &   & \left.+ {\rm HSV2factor} \times {\rm HSV2} \right)
    \end{eqnarray}


Note that this is again a time dependent exponential hazard of the form

.. math::
    {\rm hazard} = \exp(A+Bt)

In the formula, :math:`G` is a value related to the gender of the person, 0 for a man and
1 for a woman. The number :math:`P` represents the number of partners of the person that
are both HIV infected and diagnosed. The value of :math:`D` is an indication of whether
the person was diagnosed previously: its value is 0 if this is the initial diagnosis event, or
1 if it's a re-diagnosis (after :ref:`dropping out <dropout>` of treatment). The value of :math:`HSV2` is an indication of whether the person is infected with HSV2: its value is 0 if the person is not infected with HSV2 and 1 if the person is infected with HSV2.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``diagnosis.baseline`` (0): |br|
   Controls the corresponding :math:`{\rm baseline}` value in the expression for the hazard.
 - ``diagnosis.agefactor`` (0): |br|
   Controls the corresponding :math:`{\rm agefactor}` value in the expression for the hazard.
   This allows one to let the age of a person influence the hazard.
 - ``diagnosis.genderfactor`` (0): |br|
   Controls the :math:`{\rm genderfactor}` parameter in the hazard. This allows you
   to have a different hazard depending on the gender of the person.
 - ``diagnosis.diagpartnersfactor`` (0): |br|
   Corresponds to the value of :math:`{\rm diagpartnersfactor}` in the expression for the
   hazard. The idea is to allow the number of partners that have already been diagnosed
   to have an effect on a person's diagnosis time: if a person is not feeling well and
   knows that some of the partners are infected with HIV, this can be an incentive to
   go to the doctor sooner.
 - ``diagnosis.isdiagnosedfactor`` (0): |br|
   Using this :math:`{\rm isdiagnosedfactor}` value in the hazard, it is possible to
   have a different hazard if the person was diagnosed before. After :ref:`dropping out <dropout>`
   of treatment, for example because a person is feeling better and no longer feels
   the need for treatment, a diagnosis event will be scheduled again. It is reasonable
   to think that a person may go to the doctor again sooner when he already knows
   about the HIV infection.
 - ``diagnosis.beta`` (0): |br|
   Corresponds to the :math:`{\beta}` factor in the hazard expression, allowing one to
   take the time since infection into account.
 - ``diagnosis.HSV2factor`` (0): |br|
   Using the :math:`{\rm HSV2factor}`, it is possible to have a different hazard when the person is infected with HSV2.
 - ``diagnosis.t_max`` (200): |br|
   As explained in the section about :ref:`'time limited' hazards <timelimited>`, an
   exponential function needs some kind of threshold value (after which it stays
   constant) to be able to perform the necessary calculations. This configuration
   value is a measure of this threshold.

.. _dissolution:
    
Dissolution event
^^^^^^^^^^^^^^^^^

As soon as a :ref:`relationship is formed <formation>` a dissolution event gets scheduled
to allow for the possibility that the relationship terminates. The hazard for this
event is the following:

.. math::

    \begin{eqnarray}
        {\rm hazard} & = & \exp\left(
                      \alpha_0
                    + \alpha_1 P_{\rm man} + \alpha_2 P_{\rm woman} + \alpha_3 | P_{\rm woman} - P_{\rm man}| \right. \\
            & &       
                    + \alpha_4 \left(\frac{(t-t_{\rm birth,man}) + (t-t_{\rm birth,woman})}{2}\right) \\
            & & \left.
                    + \alpha_5 | (t-t_{\rm birth,man}) - (t-t_{\rm birth,woman}) - D_{\rm pref} |
                    + \beta (t - t_{\rm ref})  
               \right) 
    \end{eqnarray}

Note that this is again a time dependent exponential hazard of the form

.. math::

    {\rm hazard} = \exp(A+Bt)

In this expression, :math:`P_{\rm man}` and :math:`P_{\rm woman}` are the number of partners
the man and woman in the relationship have. The value :math:`D_{\rm pref}` represents
the preferred age difference between a man and a woman. The value of :math:`t_{\rm ref}` is the
time at which the relationship was formed.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``dissolution.alpha_0`` (0.1): |br|
   The value of :math:`\alpha_0` in the expression for the hazard, allowing one to establish
   a baseline value.
 - ``dissolution.alpha_1`` (0): |br|
   The value of :math:`\alpha_1` in the hazard formula, corresponding to a weight for the
   number of relationships the man in the relationship has.
 - ``dissolution.alpha_2`` (0): |br|
   The value of :math:`\alpha_2` in the hazard formula, corresponding to a weight for the
   number of relationships the woman in the relationship has.
 - ``dissolution.alpha_3`` (0): |br|
   The value of :math:`\alpha_3` in the hazard expression, by which the influence of the
   difference in number of partners can be specified.
 - ``dissolution.alpha_4`` (0): |br|
   The value of :math:`\alpha_4` in the expression for the hazard, a weight for the average
   age of the partners.
 - ``dissolution.alpha_5`` (0): |br|
   The factor :math:`\alpha_5` controls the relative importance of how much the age gap
   between man and woman differs from the preferred age difference :math:`D_{\rm pref}`.
 - ``dissolution.Dp`` (0): |br|
   This configures the preferred age difference :math:`D_{\rm pref}` in the hazard
   expression. Note that to take this into account, :math:`\alpha_5` should also be
   set to a non-zero value.
 - ``dissolution.beta`` (0): |br|
   As can be seen in the expression for the hazard, using this value the 'age'
   of the relationship can be taken into account.
 - ``dissolution.t_max`` (200): |br|
   As explained in the section about :ref:`'time limited' hazards <timelimited>`, an
   exponential function needs some kind of threshold value (after which it stays
   constant) to be able to perform the necessary calculations. This configuration
   value is a measure of this threshold.

.. _dissolutionmsm:
    
MSM Dissolution event
^^^^^^^^^^^^^^^^^^^^^

As soon as an :ref:`MSM relationship is formed <formationmsm>` an MSM dissolution event 
gets scheduled to allow for the possibility that the relationship terminates. The 
hazard for this event is the following:

.. math::

    \begin{eqnarray}
        {\rm hazard} & = & \exp\left(
                      \alpha_0
                    + \alpha_{12} ( P_{\rm man1} + P_{\rm man2} ) + \alpha_3 | P_{\rm man1} - P_{\rm man2}| \right. \\
            & &       
                    + \alpha_4 \left(\frac{(t-t_{\rm birth,man1}) + (t-t_{\rm birth,man2})}{2}\right) \\
            & & \left.
                    + \alpha_5 | (t-t_{\rm birth,man1}) - (t-t_{\rm birth,man2}) |
                    + \beta (t - t_{\rm ref})  
               \right) 
    \end{eqnarray}

Note that this is again a time dependent exponential hazard of the form

.. math::
    
    {\rm hazard} = \exp(A+Bt)


In this expression, :math:`P_{\rm man1}` and :math:`P_{\rm man2}` are the number of partners
the men in the relationship have. The value of :math:`t_{\rm ref}` is the
time at which the relationship was formed.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``dissolutionmsm.alpha_0`` (0.1): |br|
   The value of :math:`\alpha_0` in the expression for the hazard, allowing one to establish
   a baseline value.
 - ``dissolutionmsm.alpha_12`` (0): |br|
   The value of :math:`\alpha_{12}` in the hazard formula, corresponding to a weight for the
   number of relationships the men in the relationship have.
 - ``dissolutionmsm.alpha_3`` (0): |br|
   The value of :math:`\alpha_3` in the hazard expression, by which the influence of the
   difference in number of partners can be specified.
 - ``dissolutionmsm.alpha_4`` (0): |br|
   The value of :math:`\alpha_4` in the expression for the hazard, a weight for the average
   age of the partners.
 - ``dissolutionmsm.alpha_5`` (0): |br|
   The factor :math:`\alpha_5` controls the relative importance of the age gap
   between the men in the relationship.
 - ``dissolutionmsm.beta`` (0): |br|
   As can be seen in the expression for the hazard, using this value the 'age'
   of the relationship can be taken into account.
 - ``dissolutionmsm.t_max`` (200): |br|
   As explained in the section about :ref:`'time limited' hazards <timelimited>`, an
   exponential function needs some kind of threshold value (after which it stays
   constant) to be able to perform the necessary calculations. This configuration
   value is a measure of this threshold.

.. _dropout:

ART treatment dropout event
^^^^^^^^^^^^^^^^^^^^^^^^^^^

When a :ref:`monitoring event <monitoring>` gets triggered and the person is both eligible
and willing to receive treatment, treatment is started causing the set-point viral
load of the person to be lowered. When treatment starts, a dropout event is scheduled
as well, to allow a person to drop out of treatment.

Currently, the dropout event is not hazard-based, instead a random number is
picked from a one dimensional probability distribution as specified in
``dropout.interval.dist.type`` and related configuration options.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``dropout.interval.dist.type`` ('uniform' by default, with boundaries of 3 months and 10 years): |br|
   Using this configuration option you can specify the probability distribution to
   use when obtaining the time after which a person will drop out of treatment. By
   default, this is a uniform distribution with equal non-zero probability between 3 months and
   10 years, and zero otherwise. :ref:`Other distributions <prob1d>` can be specified as well, as
   explained previously.

.. _formation:

Formation event
^^^^^^^^^^^^^^^

Depending on the :ref:`'eyecap' <eyecap>` setting, for a number of man/woman pairs,
formation events will be scheduled. When such an event fires, a relationship
between these two persons will be formed. Apart from scheduling a 
:ref:`dissolution event <dissolution>`, a :ref:`conception event <conception>`
will get scheduled if the woman in the relationship is not yet pregnant,
and in case just one of the partners is infected with HIV, a
:ref:`transmission event <transmission>` will be scheduled as well.

The formation event is hazard based, and there are currently three hazard types
that can be used by configuring ``formation.hazard.type``. The first hazard type
is called the :ref:`'simple' <simplehazard>` hazard, and is nearly equal to the hazard of the
:ref:`dissolution event <dissolution>`. 
The second hazard type, called :ref:`'agegap' <agegaphazard>`, is more advanced. Not only can the
preferred age gap differ from one person to the next, but there's also an
age dependent component in this preferred age gap. The third hazard, :ref:`'agegapry' <agegapryhazard>`
allows for the weight of the agegap terms to be age dependent. To make this possible,
the time dependency in the age gap part of the hazard is only approximate: times
will refer to a reference year (hence the 'ry' in the hazard name) which can be
set using the :ref:`'synchronize reference year' <syncrefyear>` event.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``formation.hazard.type`` (``agegap``): |br|
   This parameter specifies which formation hazard will be used. Allowed values
   are ``simple``, ``agegap`` and ``agegapry``.

.. _simplehazard:

The ``simple`` formation hazard
"""""""""""""""""""""""""""""""

The hazard for the ``simple`` formation event is shown further on and is nearly identical
to the one of the :ref:`dissolution event <dissolution>`. Apart from a few extra terms
in the expression for the hazard, the most important difference
is the factor :math:`F` in front. 

.. _formationnorm:

This factor :math:`F` is a normalization factor which takes the population size (or 
more accurately the size when viewed through the
the :ref:`'eyecaps' <eyecap>`) into account.
This is necessary because the number of formation events that get scheduled is
proportional to the population size (for a fixed 'eyecap' fraction). So if no
normalization factor would be used, a larger population would automatically mean
that more relationships are formed. By roughly dividing the hazard by the population
size, this increase in available possible relationships when the population size
is larger, does not automatically result in more relationships anymore.

To be very accurate, each increase or decrease in the population size (by a birth
or death of a person) should be taken into account, and to do so all formation event
fire times would need to be recalculated according to the changed (because :math:`F` changed)
hazard. This would become a huge bottleneck, especially when the population size
is large, and birth and mortality events occur frequently.

To work around this bottleneck, it is not the true population size that is used in
this normalization factor, but the last known population size. This last known size
can be updated by the event that :ref:`synchronizes population statistics <syncpopstats>`,
at which time all formation event fire times will be recalculated. If the population
size stays roughly constant, this is not necessary, but it will be if the population
size grows or shrinks considerably during the simulation.

The hazard for this formation type is the following:

.. math::

    \begin{eqnarray}
        {\rm hazard} & = & F \times \exp\left(
                      \alpha_0
                    + \alpha_1 P_{\rm man} + \alpha_2 P_{\rm woman} + \alpha_3 | P_{\rm woman} - P_{\rm man}| \right. \\
            & &     + \alpha_4 \left(\frac{(t-t_{\rm birth,man}) + (t-t_{\rm birth,woman})}{2}\right) \\
            & &     + \alpha_5 | (t-t_{\rm birth,man}) - (t-t_{\rm birth,woman}) - D_{\rm pref} | \\
            & &     + \alpha_6 (E_{\rm man} + E_{\rm woman}) + \alpha_7 |E_{\rm man} - E_{\rm woman}| \\
	    & &     + \alpha_{\rm dist} |\vec{R}_{\rm man} - \vec{R}_{\rm woman}| \\
            & &     
                    + \beta (t - t_{\rm ref})  
	       \left. \right) 
    \end{eqnarray}

Note that this is again a time dependent exponential hazard of the form

.. math::

    {\rm hazard} = \exp(A+Bt)

In this expression, :math:`P_{\rm man}` and :math:`P_{\rm woman}` are the number of partners
the man and woman in the relationship have. The value :math:`D_{\rm pref}` represents
the preferred age difference between a man and a woman, and :math:`E_{\rm man}` and
:math:`E_{\rm woman}` are parameters that can be different for each person describing
their :ref:`eagerness <eagerness>` of forming a relationship. 
The distance between
the :ref:`locations <geodist>` :math:`\vec{R}_{\rm man}` and :math:`\vec{R}_{\rm woman}` of the partners 
involved can be taken into account as well.

The value of :math:`t_{\rm ref}` is the time
at which the relationship between the two persons became possible. If no relationship
existed between the two people earlier, this is the time at which the youngest person
reached the :ref:`debut <debut>` age. On the other hand, if a relationship between
these partners did exist before, it is the time at which that relationship got
:ref:`dissolved <dissolution>`. The factor :math:`F` is the normalization factor discussed earlier.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``formation.hazard.simple.alpha_0`` (0.1): |br|
   The value of :math:`\alpha_0` in the expression for the hazard, allowing one to establish
   a baseline value.
 - ``formation.hazard.simple.alpha_1`` (0): |br|
   The value of :math:`\alpha_1` in the hazard formula, corresponding to a weight for the
   number of relationships the man in the relationship has.
 - ``formation.hazard.simple.alpha_2`` (0): |br|
   The value of :math:`\alpha_2` in the hazard formula, corresponding to a weight for the
   number of relationships the woman in the relationship has.
 - ``formation.hazard.simple.alpha_3`` (0): |br|
   The value of :math:`\alpha_3` in the hazard expression, by which the influence of the
   difference in number of partners can be specified.
 - ``formation.hazard.simple.alpha_4`` (0): |br|
   The value of :math:`\alpha_4` in the expression for the hazard, a weight for the average
   age of the partners.
 - ``formation.hazard.simple.alpha_5`` (0): |br|
   The factor :math:`\alpha_5` controls the relative importance of how much the age gap
   between man and woman differs from the preferred age difference :math:`D_{\rm pref}`.
 - ``formation.hazard.simple.alpha_6`` (0): |br|
   Weight for the sum of the :ref:`eagerness <eagerness>` parameters of both partners.
 - ``formation.hazard.simple.alpha_7`` (0): |br|
   Weight for the difference of the :ref:`eagerness <eagerness>` parameters of both partners.
 - ``formation.hazard.simple.alpha_dist`` (0): |br|
   This configures the weight :math:`\alpha_{\rm dist}` of the geographical distance
   between the partners.
 - ``formation.hazard.simple.Dp`` (0): |br|
   This configures the preferred age difference :math:`D_{\rm pref}` in the hazard
   expression. Note that to take this into account, :math:`\alpha_5` should also be
   set to a non-zero value.
 - ``formation.hazard.simple.beta`` (0): |br|
   Corresponds to :math:`\beta` in the hazard expression and allows you to take the
   time since the relationship became possible into account.
 - ``formation.hazard.simple.t_max`` (200): |br|
   As explained in the section about :ref:`'time limited' hazards <timelimited>`, an
   exponential function needs some kind of threshold value (after which it stays
   constant) to be able to perform the necessary calculations. This configuration
   value is a measure of this threshold.

.. _agegaphazard:

The ``agegap`` formation hazard
"""""""""""""""""""""""""""""""

The ``agegap`` formation hazard is more complex than the previous hazard, providing
some additional functionality. With this hazard it's possible to simulate the
previous ``simple`` hazard, but not the other way around. The general look of the
hazard is the same as before, but with some important differences:

.. math::

    \begin{array}{lll}
		{\rm hazard} & = & F \times \exp\left( \alpha_{\rm baseline} + \alpha_{\rm numrel,man} P_{\rm man} + \alpha_{\rm numrel,woman} P_{\rm woman} \right. \\
          & + & \alpha_{\rm numrel,diff}|P_{\rm man} - P_{\rm woman}| \\
	      & + & \alpha_{\rm meanage} \left(\frac{A_{\rm man}(t)+A_{\rm woman}(t)}{2}\right)  \\
          & + & \alpha_{\rm eagerness,sum}(E_{\rm man} + E_{\rm woman}) +
                \alpha_{\rm eagerness,diff}|E_{\rm man} - E_{\rm woman}| \\
	  & + & \alpha_{\rm dist} |\vec{R}_{\rm man} - \vec{R}_{\rm woman}| \\
		  &	+ & \alpha_{\rm gap,factor,man} |A_{\rm man}(t)-A_{\rm woman}(t)-D_{p,{\rm man}}-\alpha_{\rm gap,agescale,man} A_{\rm man}(t)| \\
		  & + & \alpha_{\rm gap,factor,woman} |A_{\rm man}(t)-A_{\rm woman}(t)-D_{p,{\rm woman}}-\alpha_{\rm gap,agescale,woman} A_{\rm woman}(t)| \\
		  & + & \left. \beta (t-t_{\rm ref}) \right) 
    \end{array}

In this equation the following notation is used for clarity:

.. math::

    A_{\rm man}(t) = t - t_{\rm birth,man}

.. math::

    A_{\rm woman}(t) = t - t_{\rm birth,woman}

i.e., :math:`A(t)` represents the age of someone. As you can see from the expression, it is now
possible to specify a preferred age difference on a per-person basis. This 
:ref:`personal preferred age difference <personagegap>` :math:`D_{p,{\rm man}}` 
or :math:`D_{p,{\rm woman}}` can be controlled by specifying a
one dimensional probability distribution, as explained in the person settings.
Apart from more variation in the age gap, the preferred age gap for a man or a woman
can also vary in time, based on the age of one of the partners. The importance of
such a change can be controlled using the :math:`\alpha_{\rm gap,agescale,man}` and
:math:`\alpha_{\rm gap,agescale,woman}` parameters.

In front of the hazard, there is again a factor :math:`F`, just like with the ``simple``
hazard. As explained :ref:`there <formationnorm>`, it serves as a normalization factor
and for a population which can change much in size, an event that :ref:`synchronizes population statistics <syncpopstats>`
may be important to schedule regularly. 
The values :math:`P_{\rm man}` and :math:`P_{\rm woman}` are the number of partners
the man and woman in the relationship have, and :math:`E_{\rm man}` and
:math:`E_{\rm woman}` are parameters that can be different for each person describing
their :ref:`eagerness <eagerness>` of forming a relationship.
The distance between
the :ref:`locations <geodist>` :math:`\vec{R}_{\rm man}` and :math:`\vec{R}_{\rm woman}` of the partners 
involved can be taken into account as well.

As with the ``simple`` hazard, the value of :math:`t_{\rm ref}` is the time
at which the relationship between the two persons became possible. If no relationship
existed between the two people earlier, this is the time at which the youngest person
reached the :ref:`debut <debut>` age. On the other hand, if a relationship between
these partners did exist before, it is the time at which that relationship got
:ref:`dissolved <dissolution>`. 

Calculating the mapping from internal time to real-world time for this hazard
can no longer be done using the exponential time dependent hazard we encountered
e.g. in the ``simple`` hazard. The reason is the time dependence that is now present
inside the terms with the absolute values. To see how the mapping is done in this
case, you can look at the calculations in this document: 
`age gap hazard calculations <_static/formationhazard_agegap.pdf>`_

The following IPython notebook provides some simple examples of this ``agegap``
hazard: `agegap_hazard_examples.ipynb <_static/agegap_hazard_examples.ipynb>`_.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``formation.hazard.agegap.baseline`` (0.1): |br|
   The value of :math:`\alpha_{\rm baseline}` in the expression for the hazard, allowing one to establish
   a baseline value.
 - ``formation.hazard.agegap.numrel_man`` (0): |br|
   The value of :math:`\alpha_{\rm numrel,man}` in the hazard formula, corresponding to a weight for the
   number of relationships the man in the relationship has.
 - ``formation.hazard.agegap.numrel_woman`` (0): |br|
   The value of :math:`\alpha_{\rm numrel,woman}` in the hazard formula, corresponding to a weight for the
   number of relationships the woman in the relationship has.
 - ``formation.hazard.agegap.numrel_diff`` (0): |br|
   The value of :math:`\alpha_{\rm numrel,diff}` in the hazard expression, by which the influence of the
   difference in number of partners can be specified.
 - ``formation.hazard.agegap.meanage`` (0): |br|
   The value of :math:`\alpha_{\rm meanage}` in the expression for the hazard, a weight for the average
   age of the partners.
 - ``formation.hazard.agegap.eagerness_sum`` (0): |br|
   Weight :math:`\alpha_{\rm eagerness,sum}` for the sum of the :ref:`eagerness <eagerness>` parameters of both partners.
 - ``formation.hazard.agegap.eagerness_diff`` (0): |br|
   Weight :math:`\alpha_{\rm eagerness,diff}` for the difference of the :ref:`eagerness <eagerness>` parameters of both partners.
 - ``formation.hazard.agegap.gap_factor_man`` (0): |br|
   With this setting you set :math:`\alpha_{\rm gap,factor,man}`, specifying the influence of the
   age gap from the man's point of view.
 - ``formation.hazard.agegap.gap_agescale_man`` (0): |br|
   This controls :math:`\alpha_{\rm gap,agescale,man}`, which allows you to vary the preferred age
   gap with the age of the man in the relationship.
 - ``formation.hazard.agegap.gap_factor_woman`` (0): |br|
   With this setting you set :math:`\alpha_{\rm gap,factor,woman}`, specifying the influence of the
   age gap from the man's point of view.
 - ``formation.hazard.agegap.gap_agescale_woman`` (0): |br|
   This controls :math:`\alpha_{\rm gap,agescale,woman}`, which allows you to vary the preferred age
   gap with the age of the woman in the relationship.
 - ``formation.hazard.agegap.distance`` (0): |br|
   This configures the weight :math:`\alpha_{\rm dist}` of the geographical distance
   between the partners.
 - ``formation.hazard.agegap.beta`` (0): |br|
   Corresponds to :math:`\beta` in the hazard expression and allows you to take the
   time since the relationship became possible into account.
 - ``formation.hazard.agegap.t_max`` (200): |br|
   Even though this hazard is no longer a simple time dependent exponential,
   it will still be necessary to provide some kind of cut-off, as explained in
   the section about :ref:`'time limited' hazards <timelimited>`. This configuration
   value is a measure of this threshold.

.. _agegapryhazard:

The ``agegapry`` formation hazard
"""""""""""""""""""""""""""""""""

While the :ref:`'agegap' <agegaphazard>` hazard already allows for a great deal of
flexibility, it is not possible to vary the importance of the age gap terms as
people get older. The following hazard is very similar to the ``agegap`` one,
but this does allow the importance of the age gap to change over time:

.. math::

    \begin{array}{lll}
		{\rm hazard} & = & F \times \exp\left( \alpha_{\rm baseline} \right. \\
          & + & \alpha_{\rm numrel,man} P_{\rm man} ( 1 + \alpha_{\rm numrel,scale,man} g_{\rm man}(t_{\rm ry}) ) \\
          & + & \alpha_{\rm numrel,woman} P_{\rm woman} ( 1 + \alpha_{\rm numrel,scale,woman} g_{\rm woman}(t_{\rm ry}) )\\
          & + & \alpha_{\rm numrel,diff}|P_{\rm man} - P_{\rm woman}| \\
	      & + & \alpha_{\rm meanage} \left(\frac{A_{\rm man}(t)+A_{\rm woman}(t)}{2}\right)  \\
	  & + & \alpha_{\rm dist} |\vec{R}_{\rm man} - \vec{R}_{\rm woman}| \\
          & + & \alpha_{\rm eagerness,sum}(E_{\rm man} + E_{\rm woman}) +
                \alpha_{\rm eagerness,diff}|E_{\rm man} - E_{\rm woman}| \\
		  &	+ & G_{\rm man}(t_{\rm ry}) + G_{\rm woman}(t_{\rm ry}) \\
		  & + & \left. \beta (t-t_{\rm ref}) \right) 
    \end{array}

In this equation, the terms :math:`G_{\rm man}` and :math:`G_{\rm woman}` for the age gap
between partners in a relationship is given by the following expressions:

.. math::

    \begin{array}{lll}
        G_{\rm man}(t_{\rm ry}) & = & \left[\alpha_{\rm gap,factor,man,const} + 
                                      \alpha_{\rm gap,factor,man,exp} 
                                       \exp\left( \alpha_{\rm gap,factor,man,age} \left( A_{\rm man}(t_{\rm ry}) - A_{\rm debut} \right) \right) \right]\\ 
                                & & \times |g_{\rm man}(t_{\rm ry})|
    \end{array}

.. math::

    \begin{array}{lll}
        G_{\rm woman}(t_{\rm ry}) & = & \left[\alpha_{\rm gap,factor,woman,const} + 
                                      \alpha_{\rm gap,factor,woman,exp} 
                                       \exp\left( \alpha_{\rm gap,factor,woman,age} \left( A_{\rm woman}(t_{\rm ry}) - A_{\rm debut} \right) \right) \right]\\ 
                                & & \times |g_{\rm woman}(t_{\rm ry})|
    \end{array}

where :math:`g_{\rm man}(t_{\rm ry})` and :math:`g_{\rm woman}(t_{\rm ry})` specify the
preferred age gaps themselves, which can change over time:

.. math::

    g_{\rm man}(t_{\rm ry}) = 
          A_{\rm man}(t_{\rm ry})-A_{\rm woman}(t_{\rm ry})-D_{p,{\rm man}}-\alpha_{\rm gap,agescale,man} A_{\rm man}(t_{\rm ry})


.. math::

    g_{\rm woman}(t_{\rm ry}) =
          A_{\rm man}(t_{\rm ry})-A_{\rm woman}(t_{\rm ry})-D_{p,{\rm woman}}-\alpha_{\rm gap,agescale,woman} A_{\rm woman}(t_{\rm ry})

In these equations again the following notation is used:

.. math::

    A_{\rm man}(t) = t - t_{\rm birth,man}

.. math::

    A_{\rm woman}(t) = t - t_{\rm birth,woman}

i.e., :math:`A(t)` represents the age of someone. 
Looking at these full age gap terms :math:`G_{\rm man}` and :math:`G_{\rm woman}`, you can see 
that they are similar to the ones from
the ``agegap`` hazard, but the prefactor is no longer a simple configurable constant.
By tuning several parameters, the importance of these age gap terms can now be made
age-dependent.

However, this age-dependence is in fact only approximate because :math:`t_{\rm ry}`
is used in these expressions instead of the simulation time :math:`t`: to reduce the complexity
of the hazard and keep the performance up, inside the age gap terms, strict time
depencency on :math:`t` is replaced by approximate time dependency on a reference year
:math:`t_{\rm ry}`. By only changing this reference time at certain intervals (see
the :ref:`reference year synchronization event <syncrefyear>`), the time dependency
of the hazard becomes much more straightforward. Note that certain terms still
have a dependency on the simulation time :math:`t`, causing this hazard to be of the
form :math:`\exp(A + Bt)`.

By setting :math:`\alpha_{\rm numrel,scale,man}` or :math:`\alpha_{\rm numrel,scale,woman}`,
using the same approximation the importance of the number of partners can be
made dependent on the age gap. The meaning of the other quantities in the hazard 
is the same as in the :ref:`'agegap' <agegaphazard>`
hazard. 

An IPython notebook that illustrates how a funnel-like distribution of the
formed relationships can be generated using this ``agegapry`` hazard, can be found
here: `agegapry_hazard_funnel.ipynb <_static/agegapry_hazard_funnel.ipynb>`_.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``formation.hazard.agegapry.baseline`` (0.1): |br|
   The value of :math:`\alpha_{\rm baseline}` in the expression for the hazard, allowing one to establish
   a baseline value.
 - ``formation.hazard.agegapry.numrel_man`` (0): |br|
   The value of :math:`\alpha_{\rm numrel,man}` in the hazard formula, corresponding to a weight for the
   number of relationships the man in the relationship has.
 - ``formation.hazard.agegapry.numrel_scale_man`` (0): |br|
   The value of :math:`\alpha_{\rm numrel,scale,man}` in the formula for the hazard.
 - ``formation.hazard.agegapry.numrel_woman`` (0): |br|
   The value of :math:`\alpha_{\rm numrel,woman}` in the hazard formula, corresponding to a weight for the
   number of relationships the woman in the relationship has.
 - ``formation.hazard.agegapry.numrel_scale_woman`` (0): |br|
   The value of :math:`\alpha_{\rm numrel,scale,woman}` in the formula for the hazard.
 - ``formation.hazard.agegapry.numrel_diff`` (0): |br|
   The value of :math:`\alpha_{\rm numrel,diff}` in the hazard expression, by which the influence of the
   difference in number of partners can be specified.
 - ``formation.hazard.agegapry.meanage`` (0): |br|
   The value of :math:`\alpha_{\rm meanage}` in the expression for the hazard, a weight for the average
   age of the partners.
 - ``formation.hazard.agegapry.eagerness_sum`` (0): |br|
   Weight :math:`\alpha_{\rm eagerness,sum}` for the sum of the :ref:`eagerness <eagerness>` parameters of both partners.
 - ``formation.hazard.agegapry.eagerness_diff`` (0): |br|
   Weight :math:`\alpha_{\rm eagerness,diff}` for the difference of the :ref:`eagerness <eagerness>` parameters of both partners.
 - ``formation.hazard.agegapry.gap_factor_man_const`` (0): |br|
   The value of :math:`\alpha_{\rm gap,factor,man,const}` in the age gap term :math:`G_{\rm man}(t_{\rm ry})`.
 - ``formation.hazard.agegapry.gap_factor_man_exp`` (0): |br|
   The value of :math:`\alpha_{\rm gap,factor,man,exp}` in the age gap term :math:`G_{\rm man}(t_{\rm ry})`.
 - ``formation.hazard.agegapry.gap_factor_man_age`` (0): |br|
   The value of :math:`\alpha_{\rm gap,factor,man,age}` in the age gap term :math:`G_{\rm man}(t_{\rm ry})`.
 - ``formation.hazard.agegapry.gap_agescale_man`` (0): |br|
   This controls :math:`\alpha_{\rm gap,agescale,man}`, which allows you to vary the preferred age
   gap with the age of the man in the relationship.
 - ``formation.hazard.agegapry.gap_factor_woman_const`` (0): |br|
   The value of :math:`\alpha_{\rm gap,factor,woman,const}` in the age gap term :math:`G_{\rm woman}(t_{\rm ry})`.
 - ``formation.hazard.agegapry.gap_factor_woman_age`` (0): |br|
   The value of :math:`\alpha_{\rm gap,factor,woman,age}` in the age gap term :math:`G_{\rm woman}(t_{\rm ry})`.
 - ``formation.hazard.agegapry.gap_factor_woman_exp`` (0): |br|
   The value of :math:`\alpha_{\rm gap,factor,woman,exp}` in the age gap term :math:`G_{\rm woman}(t_{\rm ry})`.
 - ``formation.hazard.agegapry.gap_agescale_woman`` (0): |br|
   This controls :math:`\alpha_{\rm gap,agescale,woman}`, which allows you to vary the preferred age
   gap with the age of the woman in the relationship.
 - ``formation.hazard.agegapry.distance`` (0): |br|
   This configures the weight :math:`\alpha_{\rm dist}` of the geographical distance
   between the partners.
 - ``formation.hazard.agegapry.beta`` (0): |br|
   Corresponds to :math:`\beta` in the hazard expression and allows you to take the
   time since the relationship became possible into account.
 - ``formation.hazard.agegapry.t_max`` (200): |br|
   As explained in the section about :ref:`'time limited' hazards <timelimited>`, an
   exponential function needs some kind of threshold value (after which it stays
   constant) to be able to perform the necessary calculations. This configuration
   value is a measure of this threshold.
 - ``formation.hazard.agegapry.maxageref.diff`` (1): |br|
   As explained above, the age gap terms do not use the real time dependency :math:`t`, but
   refer to a reference time :math:`t_{\rm ry}` that needs to be synchronized periodically
   using the :ref:`synchronize reference year event <syncrefyear>`. The program will abort
   if it detects that the last reference time synchronization was more than this
   amount of time ago, which by default is one year.

.. _formationmsm:

MSM Formation event
^^^^^^^^^^^^^^^^^^^

The MSM formation event is very similar to the :ref:`heterosexual formation event <formation>`.
If :ref:`MSM <populationmsm>` relationships are enabled in the simulation,
depending on the :ref:`'eyecap' <eyecap>` setting, MSM formation events will be scheduled
for a number of man/man pairs. When such an event fires, a relationship
between these two men will be formed. Apart from scheduling an 
:ref:`MSM dissolution event <dissolutionmsm>`, in case just one of the partners is infected 
with HIV, a :ref:`transmission event <transmission>` will be scheduled as well.

Just like the heterosexual relationship formation event, this event is also hazard
based and uses the same hazard types, which can be configured using ``formationmsm.hazard.type``:
there are the :ref:`'simple' <simplehazardmsm>` hazard, the :ref:`'agegap' <agegaphazardmsm>` hazard
and the :ref:`'agegapry' <agegapryhazardmsm>` hazard.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``formationmsm.hazard.type`` (``agegap``): |br|
   This parameter specifies which formation hazard will be used. Allowed values
   are ``simple``, ``agegap`` and ``agegapry``.

.. _simplehazardmsm:

The ``simple`` formation hazard (MSM version)
"""""""""""""""""""""""""""""""""""""""""""""

The hazard for the ``simple`` formation event is shown below and is nearly identical
to the one of the :ref:`MSM dissolution event <dissolutionmsm>`. Apart from a few extra terms
in the expression for the hazard, the most important difference
is the factor :math:`F` in front, a normalization factor which has the same meaning as in
the :ref:`corresponding hazard <formationnorm>` from the heterosexual relationship formation 
event.

The hazard for this formation type is the following:

.. math::

    \begin{eqnarray}
        {\rm hazard} & = & F \times \exp\left(
                      \alpha_0
                    + \alpha_{12} (P_{\rm man1} + P_{\rm man2}) + \alpha_3 | P_{\rm man1} - P_{\rm man2}| \right. \\
            & &     + \alpha_4 \left(\frac{(t-t_{\rm birth,man1}) + (t-t_{\rm birth,man2})}{2}\right) \\
            & &     + \alpha_5 | (t-t_{\rm birth,man1}) - (t-t_{\rm birth,man2}) | \\
            & &     + \alpha_6 (E_{\rm man1} + E_{\rm man2}) + \alpha_7 |E_{\rm man1} - E_{\rm man2}| \\
	    & &     + \alpha_{\rm dist} |\vec{R}_{\rm man1} - \vec{R}_{\rm man2}| \\
            & &     \left. 
                    + \beta (t - t_{\rm ref})  
               \right) 
    \end{eqnarray}

Note that this is again a time dependent exponential hazard of the form

.. math::

    {\rm hazard} = \exp(A+Bt)

In this expression, :math:`P_{\rm man1}` and :math:`P_{\rm man2}` are the number of partners
of the men in the relationship, and :math:`E_{\rm man1}` and
:math:`E_{\rm man2}` are parameters that can be different for each person describing
their :ref:`eagerness <eagerness>` of forming a relationship.
The distance between
the :ref:`locations <geodist>` :math:`\vec{R}_{\rm man1}` and :math:`\vec{R}_{\rm man2}` of the partners 
involved can be taken into account as well.

The value of :math:`t_{\rm ref}` is the time
at which the relationship between the two persons became possible. If no relationship
existed between the two people earlier, this is the time at which the youngest person
reached the :ref:`debut <debut>` age. On the other hand, if a relationship between
these partners did exist before, it is the time at which that relationship got
:ref:`dissolved <dissolutionmsm>`. 

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``formationmsm.hazard.simple.alpha_0`` (0.1): |br|
   The value of :math:`\alpha_0` in the expression for the hazard, allowing one to establish
   a baseline value.
 - ``formationmsm.hazard.simple.alpha_12`` (0): |br|
   The value of :math:`\alpha_{12}` in the hazard formula, corresponding to a weight for the
   number of relationships the men in the relationship have.
 - ``formationmsm.hazard.simple.alpha_3`` (0): |br|
   The value of :math:`\alpha_3` in the hazard expression, by which the influence of the
   difference in number of partners can be specified.
 - ``formationmsm.hazard.simple.alpha_4`` (0): |br|
   The value of :math:`\alpha_4` in the expression for the hazard, a weight for the average
   age of the partners.
 - ``formationmsm.hazard.simple.alpha_5`` (0): |br|
   The factor :math:`\alpha_5` controls the relative importance of the age gap
   between the partners.
 - ``formationmsm.hazard.simple.alpha_6`` (0): |br|
   Weight for the sum of the :ref:`eagerness <eagerness>` parameters of both partners.
 - ``formationmsm.hazard.simple.alpha_7`` (0): |br|
   Weight for the difference of the :ref:`eagerness <eagerness>` parameters of both partners.
 - ``formationmsm.hazard.simple.alpha_dist`` (0): |br|
   This configures the weight :math:`\alpha_{\rm dist}` of the geographical distance
   between the partners.
 - ``formationmsm.hazard.simple.beta`` (0): |br|
   Corresponds to :math:`\beta` in the hazard expression and allows you to take the
   time since the relationship became possible into account.
 - ``formationmsm.hazard.simple.t_max`` (200): |br|
   As explained in the section about :ref:`'time limited' hazards <timelimited>`, an
   exponential function needs some kind of threshold value (after which it stays
   constant) to be able to perform the necessary calculations. This configuration
   value is a measure of this threshold.

.. _agegaphazardmsm:

The ``agegap`` formation hazard (MSM version)
"""""""""""""""""""""""""""""""""""""""""""""

The ``agegap`` formation hazard is again very similar to its :ref:`heterosexual counterpart <agegaphazard>`:

.. math::

    \begin{array}{lll}
		{\rm hazard} & = & F \times \exp\left( \alpha_{\rm baseline} + \alpha_{\rm numrel,sum}( P_{\rm man1} + P_{\rm man2}) \right. \\
          & + & \alpha_{\rm numrel,diff}|P_{\rm man1} - P_{\rm man2}| \\
	      & + & \alpha_{\rm meanage} \left(\frac{A_{\rm man1}(t)+A_{\rm man2}(t)}{2}\right)  \\
          & + & \alpha_{\rm eagerness,sum}(E_{\rm man1} + E_{\rm man2}) +
                \alpha_{\rm eagerness,diff}|E_{\rm man1} - E_{\rm man2}| \\
	  & + & \alpha_{\rm dist} |\vec{R}_{\rm man1} - \vec{R}_{\rm man2}| \\
		  &	+ & \alpha_{\rm gap,factor} |A_{\rm man1}(t)-A_{\rm man2}(t)-D_{p,{\rm man1}}-\alpha_{\rm gap,agescale} A_{\rm man1}(t)| \\
		  & + & \alpha_{\rm gap,factor} |A_{\rm man2}(t)-A_{\rm man1}(t)-D_{p,{\rm man2}}-\alpha_{\rm gap,agescale} A_{\rm man2}(t)| \\
		  & + & \left. \beta (t-t_{\rm ref}) \right) 
    \end{array}

In this equation the following notation is used for clarity:

.. math::

    A_{\rm man}(t) = t - t_{\rm birth,man}

i.e., :math:`A(t)` represents the age of someone. As you can see from the expression, it is now
possible to specify a preferred age difference on a per-person basis. This 
:ref:`personal preferred age difference <personagegap>` :math:`D_{p,{\rm man1}}` or :math:`D_{p,{\rm man2}}` 
can be controlled by specifying a
one dimensional probability distribution, as explained in the person settings. 
Note that the preferred age gaps are the MSM specific agegaps.
Apart from more variation in these age gaps, the preferred age gap
can also vary in time, based on the age of one of the partners. The importance of
such a change can be controlled using the :math:`\alpha_{\rm gap,agescale}` parameter.

In front of the hazard, there is again a factor :math:`F`, just like with the `simple`
hazard. The values :math:`P_{\rm man1}` and :math:`P_{\rm man2}` are the number of partners
the men in the relationship have, and :math:`E_{\rm man1}` and
:math:`E_{\rm man2}` are parameters that can be different for each person describing
their :ref:`eagerness <eagerness>` of forming a relationship. Note that the eagerness
values used here are the MSM eagerness values.
The distance between
the :ref:`locations <geodist>` :math:`\vec{R}_{\rm man1}` and :math:`\vec{R}_{\rm man2}` of the partners 
involved can be taken into account as well.

As with the ``simple`` hazard, the value of :math:`t_{\rm ref}` is the time
at which the relationship between the two persons became possible. If no relationship
existed between the two people earlier, this is the time at which the youngest person
reached the :ref:`debut <debut>` age. On the other hand, if a relationship between
these partners did exist before, it is the time at which that relationship got
:ref:`dissolved <dissolutionmsm>`. 

Calculating the mapping from internal time to real-world time for this hazard
can no longer be done using the exponential time dependent hazard we encountered
e.g. in the ``simple`` hazard. The reason is the time dependence that is now present
inside the terms with the absolute values. More information can be found in the
documentation for the :ref:`heterosexual 'agegap' hazard <agegaphazard>`


Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``formationmsm.hazard.agegap.baseline`` (0.1): |br|
   The value of :math:`\alpha_{\rm baseline}` in the expression for the hazard, allowing one to establish
   a baseline value.
 - ``formationmsm.hazard.agegap.numrel_sum`` (0): |br|
   The value of :math:`\alpha_{\rm numrel,sum}` in the hazard formula, corresponding to a weight for the
   number of relationships the men in the relationship have.
 - ``formationmsm.hazard.agegap.numrel_diff`` (0): |br|
   The value of :math:`\alpha_{\rm numrel,diff}` in the hazard expression, by which the influence of the
   difference in number of partners can be specified.
 - ``formationmsm.hazard.agegap.meanage`` (0): |br|
   The value of :math:`\alpha_{\rm meanage}` in the expression for the hazard, a weight for the average
   age of the partners.
 - ``formationmsm.hazard.agegap.eagerness_sum`` (0): |br|
   Weight :math:`\alpha_{\rm eagerness,sum}` for the sum of the :ref:`eagerness <eagerness>` parameters of both partners.
   Note that the relevant value here is the MSM eagerness.
 - ``formationmsm.hazard.agegap.eagerness_diff`` (0): |br|
   Weight :math:`\alpha_{\rm eagerness,diff}` for the difference of the :ref:`eagerness <eagerness>` parameters of both partners.
   Note that the relevant value here is the MSM eagerness.
 - ``formationmsm.hazard.agegap.gap_factor`` (0): |br|
   This corresponds to :math:`\alpha_{\rm gap,factor}`, the weight for the age gap terms.
 - ``formationmsm.hazard.agegap.gap_agescale`` (0): |br|
   This corresponds to :math:`\alpha_{\rm gap,agescale}`, by which you can control how
   the preferred age gap changes over time.
 - ``formationmsm.hazard.agegap.distance`` (0): |br|
   This configures the weight :math:`\alpha_{\rm dist}` of the geographical distance
   between the partners.
 - ``formationmsm.hazard.agegap.beta`` (0): |br|
   Corresponds to :math:`\beta` in the hazard expression and allows you to take the
   time since the relationship became possible into account.
 - ``formationmsm.hazard.agegap.t_max`` (200): |br|
   Even though this hazard is no longer a simple time dependent exponential,
   it will still be necessary to provide some kind of cut-off, as explained in
   the section about :ref:`'time limited' hazards <timelimited>`. This configuration
   value is a measure of this threshold.

.. _agegapryhazardmsm:

The ``agegapry`` formation hazard (MSM version)
"""""""""""""""""""""""""""""""""""""""""""""""

The ``agegapry`` formation hazard is again very similar to its :ref:`heterosexual counterpart <agegapryhazard>`:

.. math::

    \begin{array}{lll}
		{\rm hazard} & = & F \times \exp\left( \alpha_{\rm baseline} \right. \\
          & + & \alpha_{\rm numrel,sum} [ P_{\rm man1} ( 1 + \alpha_{\rm numrel,scale} g_{\rm man1}(t_{\rm ry}) ) 
                                       +  P_{\rm man2} ( 1 + \alpha_{\rm numrel,scale} g_{\rm man2}(t_{\rm ry}) ) ] \\
          & + & \alpha_{\rm numrel,diff}|P_{\rm man1} - P_{\rm man2}| \\
	      & + & \alpha_{\rm meanage} \left(\frac{A_{\rm man1}(t)+A_{\rm man2}(t)}{2}\right)  \\
	  & + & \alpha_{\rm dist} |\vec{R}_{\rm man1} - \vec{R}_{\rm man2}| \\
          & + & \alpha_{\rm eagerness,sum}(E_{\rm man1} + E_{\rm man2}) +
                \alpha_{\rm eagerness,diff}|E_{\rm man1} - E_{\rm man2}| \\
		  &	+ & G_{\rm man1}(t_{\rm ry}) + G_{\rm man2}(t_{\rm ry}) \\
		  & + & \left. \beta (t-t_{\rm ref}) \right) 
    \end{array}

In this equation, the terms :math:`G_{\rm man1}` and :math:`G_{\rm man2}` for the age gap
between partners in a relationship is given by the following expressions:

.. math::

    \begin{array}{lll}
        G_{\rm man1}(t_{\rm ry}) & = & \left[\alpha_{\rm gap,factor,const} + 
                                      \alpha_{\rm gap,factor,exp} 
                                       \exp\left( \alpha_{\rm gap,factor,age} \left( A_{\rm man1}(t_{\rm ry}) - A_{\rm debut} \right) \right) \right]\\ 
                                & & \times |g_{\rm man1}(t_{\rm ry})|
    \end{array}

.. math::

    \begin{array}{lll}
        G_{\rm man2}(t_{\rm ry}) & = & \left[\alpha_{\rm gap,factor,const} + 
                                      \alpha_{\rm gap,factor,exp} 
                                       \exp\left( \alpha_{\rm gap,factor,age} \left( A_{\rm man2}(t_{\rm ry}) - A_{\rm debut} \right) \right) \right]\\ 
                                & & \times |g_{\rm man2}(t_{\rm ry})|
    \end{array}

where :math:`g_{\rm man1}(t_{\rm ry})` and :math:`g_{\rm man2}(t_{\rm ry})` specify the
preferred age gaps themselves, which can change over time:

.. math::

    g_{\rm man1}(t_{\rm ry}) = 
          A_{\rm man1}(t_{\rm ry})-A_{\rm man2}(t_{\rm ry})-D_{p,{\rm man1}}-\alpha_{\rm gap,agescale} A_{\rm man1}(t_{\rm ry})

.. math::

    g_{\rm man2}(t_{\rm ry}) = 
          A_{\rm man2}(t_{\rm ry})-A_{\rm man1}(t_{\rm ry})-D_{p,{\rm man2}}-\alpha_{\rm gap,agescale} A_{\rm man2}(t_{\rm ry})

In these equations again the following notation is used:

.. math::

    A_{\rm man1}(t) = t - t_{\rm birth,man1}

.. math::

    A_{\rm man2}(t) = t - t_{\rm birth,man2}

i.e., :math:`A(t)` represents the age of someone. As with the :ref:`heterosexual 'agegapry' hazard <agegapryhazard>`,
the age gap prefactor is no longer a simple configurable constant:
by tuning several parameters, the importance of these age gap terms can now be made
age-dependent.

However, this age-dependence is again approximate because :math:`t_{\rm ry}`
is used in these expressions instead of the simulation time :math:`t`: see the
documentation of its :ref:`counterpart <agegapryhazard>` for more info.
By setting :math:`\alpha_{\rm numrel,scale}` using the same approximation the 
importance of the number of partners can be made dependent on the age 
gap. The meaning of the other quantities in the hazard 
is the same as in the :ref:`'agegap' <agegaphazardmsm>` hazard. 

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``formationmsm.hazard.agegapry.baseline`` (0.1): |br|
   The value of :math:`\alpha_{\rm baseline}` in the expression for the hazard, allowing one to establish
   a baseline value.
 - ``formationmsm.hazard.agegapry.numrel_sum`` (0): |br|
   The value of :math:`\alpha_{\rm numrel,sum}` in the hazard formula, corresponding to a weight for the
   number of relationships the men in the relationship have.
 - ``formationmsm.hazard.agegapry.numrel_scale`` (0): |br|
   The value of :math:`\alpha_{\rm numrel,scale}` in the formula for the hazard.
 - ``formationmsm.hazard.agegapry.numrel_diff`` (0): |br|
   The value of :math:`\alpha_{\rm numrel,diff}` in the hazard expression, by which the influence of the
   difference in number of partners can be specified.
 - ``formationmsm.hazard.agegapry.meanage`` (0): |br|
   The value of :math:`\alpha_{\rm meanage}` in the expression for the hazard, a weight for the average
   age of the partners.
 - ``formationmsm.hazard.agegapry.eagerness_sum`` (0): |br|
   Weight :math:`\alpha_{\rm eagerness,sum}` for the sum of the :ref:`eagerness <eagerness>` parameters of both partners.
   Note that the relevant value here is the MSM eagerness.
 - ``formationmsm.hazard.agegapry.eagerness_diff`` (0): |br|
   Weight :math:`\alpha_{\rm eagerness,diff}` for the difference of the :ref:`eagerness <eagerness>` parameters of both partners.
   Note that the relevant value here is the MSM eagerness.
 - ``formationmsm.hazard.agegapry.gap_factor_const`` (0): |br|
   The value of :math:`\alpha_{\rm gap,factor,const}` in the age gap term :math:`G_{\rm manX}(t_{\rm ry})`.
 - ``formationmsm.hazard.agegapry.gap_factor_exp`` (0): |br|
   The value of :math:`\alpha_{\rm gap,factor,exp}` in the age gap term :math:`G_{\rm manX}(t_{\rm ry})`.
 - ``formationmsm.hazard.agegapry.gap_factor_age`` (0): |br|
   The value of :math:`\alpha_{\rm gap,factor,age}` in the age gap term :math:`G_{\rm manX}(t_{\rm ry})`.
 - ``formationmsm.hazard.agegapry.gap_agescale`` (0): |br|
   This controls :math:`\alpha_{\rm gap,agescale}`, which allows you to vary the preferred age
   gap with the age of the men in the relationship.
 - ``formationmsm.hazard.agegapry.distance`` (0): |br|
   This configures the weight :math:`\alpha_{\rm dist}` of the geographical distance
   between the partners.
 - ``formationmsm.hazard.agegapry.beta`` (0): |br|
   Corresponds to :math:`\beta` in the hazard expression and allows you to take the
   time since the relationship became possible into account.
 - ``formationmsm.hazard.agegapry.t_max`` (200): |br|
   As explained in the section about :ref:`'time limited' hazards <timelimited>`, an
   exponential function needs some kind of threshold value (after which it stays
   constant) to be able to perform the necessary calculations. This configuration
   value is a measure of this threshold.
 - ``formationmsm.hazard.agegapry.maxageref.diff`` (1): |br|
   As explained before, the age gap terms do not use the real time dependency :math:`t`, but
   refer to a reference time :math:`t_{\rm ry}` that needs to be synchronized periodically
   using the :ref:`synchronize reference year event <syncrefyear>`. The program will abort
   if it detects that the last reference time synchronization was more than this
   amount of time ago, which by default is one year.

.. _hivseeding:

HIV seeding event
^^^^^^^^^^^^^^^^^

When introducing the initial population in the simulation, the persons are not
yet infected with HIV. To start the infection, an HIV seeding event is scheduled,
and when (controlled by ``hivseed.time``) this is triggered, a certain amount of
people will be marked as HIV infected. 

To do so, only the people that
have the right age (as specified by ``hivseed.age.min`` and ``hivseed.age.max``)
and right gender (as specified by ``hivseed.gender``)
will be possible 'seeders', and depending on the setting of ``hivseed.type``
either a fixed number of people will be used, or each person will have a
certain probability of being a seeder. In case a fixed number is requested
but this number cannot be reached in the current simulation, the program
can be instructed to terminate depending on the ``hivseed.stop.short`` setting.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``hivseed.time`` (0): |br|
   This specifies the time at which the seeding event takes place; default is at
   the start of the simulation. Set to a negative value to disable HIV seeding.
 - ``hivseed.type`` ('fraction'): |br|
   This value specifies how the seeding will occur, either ``fraction`` to specify
   that each person in the possible seeders group will have a certain probability of
   being a seeder, or ``amount`` to specify that a fixed number of seeders should be
   chosen.
 - ``hivseed.age.min`` (0): |br|
   People who are possible seeders must be at least this old.
 - ``hivseed.age.max`` (1000): |br|
   People who are possible seeders must be at most this old.
 - ``hivseed.gender`` ('any'): |br|
   People who are possible seeders must have this gender. Can be either ``any`` (the
   default), ``male`` or ``female``.
 - ``hivseed.fraction`` (0.2): |br|
   This is only used if ``hivseed.type`` is set to ``fraction``, and specifies the
   probability each possible seeder has of actually becoming HIV infected.
 - ``hivseed.amount`` (1): |br|
   If ``hivseed.type`` is ``amount``, this number of people will be chosen from the
   group of possible seeders and marked as being HIV infected.
 - ``hivseed.stop.short`` ('yes'): |br|
   In case a specific amount of seeders should be chosen but this amount is
   not available (e.g. due to a too restrictive allowed age range), the
   program will terminate if this is set to ``yes``. It will continue despite not
   having the requested amount of seeders if set to ``no``.

HSV2 seeding event
^^^^^^^^^^^^^^^^^^

When introducing the initial population in the simulation, the persons are not
infected with HSV2. To start such an infection, an HSV2 seeding event is scheduled,
and when (controlled by ``hsv2seed.time``) this is triggered, a certain amount of
people will be marked as HSV2 infected. They can then pass on their infection
status through the :ref:`HSV2 transmission event <hsv2transmission>`.

If the event fires, only the people that
have the right age (as specified by ``hsv2seed.age.min`` and ``hsv2seed.age.max``)
and right gender (as specified by ``hsv2seed.gender``)
will be possible 'seeders', and depending on the setting of ``hsv2seed.type``
either a fixed number of people will be used, or each person will have a
certain probability of being a seeder. In case a fixed number is requested
but this number cannot be reached in the current simulation, the program
can be instructed to terminate depending on the ``hsv2seed.stop.short`` setting.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``hsv2seed.time`` (-1): |br|
   This specifies the time at which the seeding event takes place; default is 
   that HSV2 seeding is disabled.
 - ``hsv2seed.type`` ('fraction'): |br|
   This value specifies how the seeding will occur, either ``fraction`` to specify
   that each person in the possible seeders group will have a certain probability of
   being a seeder, or ``amount`` to specify that a fixed number of seeders should be
   chosen.
 - ``hsv2seed.age.min`` (0): |br|
   People who are possible seeders must be at least this old.
 - ``hsv2seed.age.max`` (1000): |br|
   People who are possible seeders must be at most this old.
 - ``hsv2seed.gender`` ('any'): |br|
   People who are possible seeders must have this gender. Can be either ``any`` (the
   default), ``male`` or ``female``.
 - ``hsv2seed.fraction`` (0.2): |br|
   This is only used if ``hsv2seed.type`` is set to ``fraction``, and specifies the
   probability each possible seeder has of actually becoming HSV2 infected.
 - ``hsv2seed.amount`` (1): |br|
   If ``hsv2seed.type`` is ``amount``, this number of people will be chosen from the
   group of possible seeders and marked as being HSV2 infected.
 - ``hsv2seed.stop.short`` ('yes'): |br|
   In case a specific amount of seeders should be chosen but this amount is
   not available (e.g. due to a too restrictive allowed age range), the
   program will terminate if this is set to ``yes``. It will continue despite not
   having the requested amount of seeders if set to ``no``.

.. _simulationintervention:

Simulation intervention event
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

It's possible that you'd want certain configuration values to change during a
simulation. For example, to lower the CD4 threshold that's used to decide if
a person will be offered antiretroviral treatment or not. Changing parameters
in a very general way can be done using this simulation intervention event
which, when triggered, reads files containing changed configuration settings
and applies them. 

Note that while the changed settings will certainly affect new events that are
introduced into the simulation, it will depend on the particular event type
whether events already scheduled in the system will be affected or not. If
the event has a fixed time at which it will take place, this time will _not_
be changed due to the intervention event. However, if the event is hazard-based
and parameters of the hazard were changed, then this will definitely have an
effect on the event's fire time.

Using this simulation intervention mechanism is easiest using R or Python,
and this is described next. Manually specifying this in the configuration file
is possible as well, as is described later.

Using R
"""""""

To use simulation interventions in R, for each such intervention event you need to create a
list with configuration values that you'd like to change, just as when you
create the configuration settings that you pass to the ``simpact.run`` command.
Additionally, such a list should contain an entry called ``time``, which
contains the simulation time at which the modified settings need to be
introduced. Additional interventions are of course allowed, so what you need
to pass to the ``simpact.run`` parameter called ``intervention``, is a list
of these lists.

For example, suppose that we already have some configuration settings in ``cfg``,
but that we'd like to set the CD4 threshold for treatment to 500 at simulation
time 5, and to 1000 at simulation time 10. We'd first create a list for the first
intervention event ::

    iv1 <- list()
    iv1["time"] <- 5
    iv1["monitoring.cd4.threshold"] <- 500

and also a similar list for the second intervention::

    iv2 <- list()
    iv2["time"] <- 10
    iv2["monitoring.cd4.threshold"] <- 1000

The full intervention configuration is then a list of these lists ::

    iv <- list(iv1, iv2)

which is what we'll pass to the ``simpact.run`` command::

    res <- simpact.run(cfg, "/tmp/simpacttest", intervention=iv)

Using Python
""""""""""""

To use simulation interventions in Python, for each such intervention event you need to create a
dictionary with configuration values that you'd like to change, just as when you
create the configuration settings that you pass to the ``PySimpactCyan`` ``run`` command.
Additionally, such a dictionary should contain an entry called ``time``, which
contains the simulation time at which the modified settings need to be
introduced. Additional interventions are of course allowed, so what you need
to pass to the ``run`` parameter called ``interventionConfig``, is a list
of these dictionaries.

For example, suppose that we already have some configuration settings in ``cfg``,
but that we'd like to set the CD4 threshold for treatment to 500 at simulation
time 5, and to 1000 at simulation time 10. We'd first create a dictionary for the first
intervention event ::

    iv1 = { }
    iv1["time"] = 5
    iv1["monitoring.cd4.threshold"] = 500

and also a similar dictionary for the second intervention::

    iv2 = { }
    iv2["time"] = 10
    iv2["monitoring.cd4.threshold"] = 1000

The full intervention configuration is then a list of these dictionaries ::

    iv = [iv1, iv2]

which is what we'll pass to the ``run`` command (assuming our ``PySimpactCyan``
object is called ``simpact``)::

    res = simpact.run(cfg, "/tmp/simpacttest", interventionConfig=iv)

Manual configuration
""""""""""""""""""""

In case you're using the :ref:`command line <commandline>` approach to run your
simulations and need to adjust simulation interventions manually, you can
enable this using the option ``intervention.enabled``. Note that this setting
as well as other related settings are not used when the R or Python interface
is employed. In that case, use the relevant mechanism described above.

If this intervention mechanism is enabled, you'll need to prepare one or
more files which are similar to the full config file, but which only contain
the settings that should be changed. For example, suppose that
we'd like to set the CD4 threshold for treatment to 500 at simulation
time 5, and to 1000 at simulation time 10. In that case we need two files,
one with the line ::

    monitoring.cd4.threshold = 500

and another file with the line ::

    monitoring.cd4.threshold = 1000

Let's call these files ``cd4_A.txt`` and ``cd4_B.txt`` respectively. In the
main config file, we need to make sure that the times for these intervention
events are mentioned using the ``intervention.times`` option, which should contain
a list of increasing times::

    intervention.times = 5,10

To specify which file should be inspected at each time, we need to use the
``intervention.baseconfigname`` and ``intervention.fileids`` settings. The first
one contains a template for the file name for each intervention, in which
the ``%`` sign is replaced by the corresponding string from ``intervention.fileids``.
In our example, we could set ::

    intervention.baseconfigname = cd4_%.txt
    intervention.fileids = A,B

In case the last option is left blank, the ``%`` sign will be replaced by what
was specified in ``intervention.times``, so the program would look for ``cd4_5.txt``
and ``cd4_10.txt``.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``intervention.enabled`` ('no'): |br|
   Indicates if the simulation interventions should be used at certain times.
   Possible values are ``no`` (the default) and ``yes``. The following options are
   only used in case this is set to ``yes``.
 - ``intervention.baseconfigname`` (no default): |br|
   This is only used if ``intervention.enabled`` is ``yes``, and contains a template
   for the filename that is used to store the modifications to the main config file in.
   In this template, ``%`` will be replaced by the relevant string from
   ``intervention.fileids``, or by the time in ``intervention.times`` in case the
   ``intervention.fileids`` option is left empty.
 - ``intervention.times`` (no default): |br|
   This is only used if ``intervention.enabled`` is ``yes``, and contains a comma
   separated list of times (positive and increasing) at which simulation interventions 
   should be executed.
 - ``intervention.fileids`` (no default): |br|
   This is only used if ``intervention.enabled`` is ``yes``, and should be either
   empty or a comma separated list with strings that should replace the ``%`` sign
   in ``intervention.baseconfigname``. If empty, the time from ``intervention.times``
   will be used to replace ``%`` instead.

.. _monitoring:

HIV infection monitoring event
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When a person has been :ref:`diagnosed <diagnosis>` as being infected with HIV,
monitoring events are scheduled to follow up on the progress of the disease
by inspecting the :ref:`person's CD4 count <cd4count>`. If this CD4 count is
below the threshold set in ``monitoring.cd4.threshold``, the person will be
offered antiretroviral treatment. Depending on the person's 
:ref:`willingness to accept <artacceptthreshold>` treatment, treatment will
then be started.

If treatment is started, the person's set-point viral load value will be
lowered according to the setting in ``monitoring.fraction.log_viralload``.
In this case no further monitoring events will be scheduled, but instead
the person will be at risk of :ref:`dropping out <dropout>` of treatment and
the corresponding event will be scheduled.

On the other hand, if the person's CD4 count was not below the threshold
or the person was not willing to start treatment,
a new monitoring event will be scheduled a while later. The precise interval
being used here, depends on the person's CD4 count and the configuration
settings. In ``monitoring.interval.piecewise.cd4s`` and ``monitoring.interval.piecewise.times``
you can specify comma separated lists of (increasing) CD4 values and their corresponding
intervals. If the CD4 value lies in between specified values, linear interpolation
will be used. If the CD4 count is less than the left-most value in this series,
the interval specified in ``monitoring.interval.piecewise.left`` will be used.
If it is larger than the right-most CD4 value, the interval from
``monitoring.interval.piecewise.right`` is used instead.

After dropping out of treatment, a new :ref:`diagnosis <diagnosis>` event will
be scheduled which then leads to new monitoring events. If this is the case,
the person will always be eligible for treatment, i.e. once a person has
received treatment he's always a candidate to start treatment again. Only
the person's willingness still matters then.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``monitoring.cd4.threshold`` (350.0): |br|
   This is the threshold value for a person's CD4 count: if the count is below this
   value, treatment will be offered (which the person still can :ref:`decline <artacceptthreshold>`).
 - ``monitoring.fraction.log_viralload`` (0.7): |br|
   If the person is eligible and willing to start treatment, ART will be started. The
   effect of this is that the person's set-point viral load will be lowered by this
   fraction on a logarithmic scale. Calling this fraction :math:`f`, this corresponds to
   :math:`V_{\rm sp,new} = (V_{\rm sp})^f`.
 - ``monitoring.interval.piecewise.cd4s`` ('200,350'): |br|
   This is a comma separated list of increasing CD4 values, and is used when looking
   up the monitoring interval for a certain CD4 count.
 - ``monitoring.interval.piecewise.times`` ('0.25,0.25'): |br|
   This is a comma separated list of monitoring time intervals that correspond to the
   CD4 values specified in ``monitoring.interval.piecewise.cd4s``.
 - ``monitoring.interval.piecewise.left`` (0.16666): |br|
   If the CD4 count is less than the left-most value specified in ``monitoring.interval.piecewise.cd4s``,
   then this interval is used (defaults to two months).
 - ``monitoring.interval.piecewise.right`` (0.5): |br|
   If the CD4 count is more than the right-most value specified in ``monitoring.interval.piecewise.cd4s``,
   then this interval is used (defaults to six months).

.. _mortality:

Mortality event
^^^^^^^^^^^^^^^

To make sure that everyone in the simulation has a limited lifespan, regardless
of being infected with HIV, there's always a mortality event scheduled for each
person. This time of death is based on a `Weibull distribution <http://en.wikipedia.org/wiki/Weibull_distribution>`_
and for a certain person, this is a fixed number that no longer changes during
the simulation.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``mortality.normal.weibull.shape`` (4.0): |br|
   This specifies the shape parameter of the Weibull distribution.
 - ``mortality.normal.weibull.scale`` (70.0): |br|
   This specifies a scale parameter to base the Weibull distribution on, but a
   difference between men and women still needs to be taken into account (see
   next option).
 - ``mortality.normal.weibull.genderdiff`` (5.0): |br|
   For a woman, half this value is added to the scale parameter specified in
   ``mortality.normal.weibull.scale``; for a man the same amount is subtracted.

.. _periodiclogging:

Periodic logging event
^^^^^^^^^^^^^^^^^^^^^^

In case you would like to keep track of how many people there are in the population
or how many were being treated, you can enable this event. The file specified in
``periodiclogging.outfile.logperiodic`` will be used to write the following properties
to: the time the event fired, the size of the population and the number of people
receiving treatment at that time. The interval between such logging events is controlled
using the ``periodiclogging.interval`` setting. If ``periodiclogging.starttime`` is negative
(the default), the first event will take place after the first interval has passed.
Otherwise, the first event is scheduled to take place at the specified time.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``periodiclogging.interval`` (-1): |br|
   This setting specifies the interval that is used to schedule the periodic logging
   events. When one event fires, the next one is scheduled. If this is set to a negative
   value, no further events will be scheduled.
 - ``periodiclogging.starttime`` (-1): |br|
   If negative, the first event will take place after the first interval has passed.
   If zero or positive, the first event will get executed at the corresponding time.
 - ``periodiclogging.outfile.logperiodic`` ('${SIMPACT_OUTPUT_PREFIX}periodiclog.csv'): |br|
   This specifies the file to which the logging occurs. By default, the value of
   the :ref:`config variable or environment variable <configfile>` ``SIMPACT_OUTPUT_PREFIX``
   will be prepended to ``periodiclog.csv`` to yield the complete filename.

.. _relocation:

Relocation event
^^^^^^^^^^^^^^^^

In case a non-trivial :ref:`2D location <geodist>` is assigned to each person, and the
:ref:`formation hazard <formation>` depends on the geographical distance between
possible partners or if an :ref:`'eyecap' <eyecap>` setting is used, a relocation
event may become interesting. Such an event changes the 2D location that's assigned
to a person, and writes a :ref:`log entry <locationlog>` to be able to keep track of
a person.

If enabled (see ``relocation.enabled``), the hazard used is a time-dependent exponential 
hazard:

.. math::

	{\rm hazard} = \exp[a + b \times (t - t_{\rm birth}) ]

Here, a baseline value :math:`a` can be configured using the ``relocation.hazard.a``
setting, and the age of the person can be taken into account using the value
of :math:`b` (the ``relocation.hazard.b`` setting).

The effect of a relocation is slightly different depending on the :ref:`'eyecap' <eyecap>`
fraction used. In case this is one, and everyone can have a relationship with everyone
else, this just changes the location of the person. By controlling the importance
of the geographical distance between partners in the :ref:`formation event <formation>`
hazards, this can still affect which precise relationships will be formed.
When only a fraction of the population can be seen however, triggering of the
relocation event will cause all existing scheduled formation events to get
cancelled. After choosing a new location of the person, a new set of interests
will be generated and formation events for these persons will get scheduled.

Note that existing, formed relationships are currently not affected by this
relocation event.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``relocation.enabled`` ('no'): |br|
   This controls if relocation events should be scheduled or not.
 - ``relocation.hazard.a`` (no default): |br|
   Sets the value of :math:`a` in the relocation hazard above.
 - ``relocation.hazard.b`` (no default): |br|
   Sets the value of :math:`b` in the relocation hazard above.
 - ``relocation.hazard.t_max`` (200): |br|
   As explained in the section about :ref:`'time limited' hazards <timelimited>`, an
   exponential function needs some kind of threshold value (after which it stays
   constant) to be able to perform the necessary calculations. This configuration
   value is a measure of this threshold.

.. _syncpopstats:

Synchronize population statistics
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As described in the :ref:`formation event <formation>`, it's possible that an event needs to use the
current population size in calculating its hazard. For a large population, there
will also be many birth and mortality events, and recalculating hazards for every
change in population size will slow the simulation down considerably. 

If the population size does not change much during the simulation, it may be adequate
to just use the population size at the beginning of the simulation. On the other hand,
if the number of people in the simulation tends to grow or shrink considerably, this
will be a poor approximation. In that case, this event can be useful, which allows you
to resynchronize the stored population size. This is a global event, meaning that afterwards
event fire times for _all_ scheduled events will be recalculated, so don't use this
more than necessary. 

If this event is needed, the interval between such synchronization events can be specified
using the ``syncpopstats.interval`` configuration option. When one event fires, the next
is scheduled to go off the specified time later.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``syncpopstats.interval`` (-1): |br|
   This specifies the interval between these synchronization events. A negative value
   means that it's disabled.

.. _syncrefyear:

Synchronize reference year
^^^^^^^^^^^^^^^^^^^^^^^^^^

With this event, a reference time can be saved in the simulation. This is used by
the :ref:`'agegapry' <agegapryhazard>` formation hazard and :ref:`HIV transmission hazard <transmission>`, 
to simplify the complexity of the hazards. 
Scheduling of the event can be disabled by setting it to a negative value (the default).

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``syncrefyear.interval`` (-1): |br|
   Specifies the time interval with which a reference simulation time should be saved.
   A negative value disables this.

.. _transmission:

HIV transmission event
^^^^^^^^^^^^^^^^^^^^^^

When a relationship is formed between two people of which one is HIV
infected, or when a relationship between two uninfected people exists
and one of them gets infected, an HIV transmission event is scheduled.
The hazard for this event is the following:

.. math::
	
	\begin{eqnarray}    
	{\rm hazard} & = & \exp\left(a + b V^{-c} + d_1 P_{\rm infected} + d_2 P_{\rm uninfected} \right.\\
                  & + & {\rm W} f_1 \exp( f_2 (A_{\rm woman}(t_{\rm ry}) - A_{\rm debut} ) )\\
			  & + & \left. e_1 HSV2_{\rm infected} + e_2 HSV2_{\rm uninfected} + g_1 b_{\rm 0j} + g_2 b_{\rm 1j} \right)
	\end{eqnarray}

In this hazard, the value of
:math:`V` is the :ref:`current viral load <viralloadx>` of the person, which can differ 
from the set-point viral load. The number of partners of the person
who is already infected is specified by :math:`P_{\rm infected}`, while the
number of partners of the person who will become infected when the event
fires, is :math:`P_{\rm uninfected}`. The value of :math:`{\rm W}` is :math:`1` if the uninfected
person is a woman, and 0 otherwise. By configuring the weights :math:`f_1` and :math:`f_2`,
is becomes possible to change the susceptibility of a woman depending on her
age. Note that this age is only specified approximately by using a reference
time :math:`t_{\rm ry}` instead of the actual time :math:`t`. This reference time can be
updated using the :ref:`reference year synchronization event <syncrefyear>`. :math:`{HSV2}_{\rm infected}` and :math:`{HSV2}_{\rm uninfected}` specify if the HIV-infected resp. uninfected person is infected with HSV2. The value of :math:`{HSV2}_{\rm infected}` resp. :math:`{HSV2}_{\rm uninfected}`  is 1 if the HIV-infected resp. uninfected person is infected with HSV2, and 0 otherwise. The values of :math:`b_{\rm 0j}` and :math:`b_{\rm 1j}` specify the susceptibility of the uninfected person to both diseases and HIV only respectively. Their values can be set using ``person.hiv.b0.dist.type`` resp. ``person.hiv.b1.dist.type`` from the :ref:`HIV related person settings <personhsv2opts>`.
The values :math:`a`, :math:`b`, :math:`c`, :math:`d_1`, :math:`d_2`, :math:`e_1`, :math:`e_2`, :math:`f_1`, :math:`f_2`, :math:`g_1` and :math:`g_2` can be configured as specified below;
the :math:`A_{\rm debut}` parameter is the :ref:`debut age <debut>`.

The form of this hazard was originally inspired by the article of :ref:`[Hargrove et al] <ref_hargrove>`.
The default parameters that are mentioned below are based on a fit to the
data from the :ref:`[Fraser et al] <ref_fraser>` article.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``hivtransmission.param.a`` (-1.3997): |br|
   This refers to the value of :math:`a` in the expression for the hazard,
   providing a baseline value.
 - ``hivtransmission.param.b`` (-12.0220): |br|
   This refers to the value of :math:`b` in the expression for the hazard.
   Together with the value of :math:`c`, this specifies the influence of the
   :ref:`current viral load <viralloadx>` of the infected person.
 - ``hivtransmission.param.c`` (0.1649): |br|
   This refers to the value of :math:`c` in the expression for the hazard.
   Together with the value of :math:`b`, this specifies the influence of the
   :ref:`current viral load <viralloadx>` of the infected person.
 - ``hivtransmission.param.d1`` (0): |br|
   This refers to the value of :math:`d_1` in the expression for the hazard,
   providing a weight based on the number of partners of the infected
   person.
 - ``hivtransmission.param.d2`` (0): |br|
   This refers to the value of :math:`d_2` in the expression for the hazard,
   providing a weight based on the number of partners of the uninfected
   person.
 - ``hivtransmission.param.e1`` (0): |br|
   This refers to the value of :math:`e_1` in the expression of the hazard and specifies the influence of the HIV-infected person being HSV2-infected.
 - ``hivtransmission.param.e2`` (0): |br|
   This refers to the value of :math:`e_2` in the expression of the hazard and specifies the influence of the HIV-uninfected person being HSV2-infected.
 - ``hivtransmission.param.f1`` (0): |br|
   This refers to the value of :math:`f_1` in the expression of the hazard.
 - ``hivtransmission.param.f2`` (0): |br|
   This refers to the value of :math:`f_2` in the expression of the hazard.
 - ``hivtransmission.param.g1`` (0): |br|
   This refers to the value of :math:`g_1` in the expression of the hazard. Set this parameter equal to 1 if you want to include the influence of susceptibility to both infections.
 - ``hivtransmission.param.g2`` (0): |br|
   This refers to the value of :math:`g_2` in the expression of the hazard. Set this parameter equal to 1 if you want to include the influence of susceptibility to HIV only.
 - ``hivtransmission.maxageref.diff`` (1): |br|
   As explained above, the hazard does not use the real time dependency :math:`t`, but
   refers to a reference time :math:`t_{\rm ry}` that needs to be synchronized periodically
   using the :ref:`synchronize reference year event <syncrefyear>`. The program will abort
   if it detects that the last reference time synchronization was more than this
   amount of time ago, which by default is one year.

.. _hsv2transmission:

HSV2 transmission event
^^^^^^^^^^^^^^^^^^^^^^^

When a person is HSV2 infected and and a relationship is formed, an HSV2 transmission
event will be scheduled. A time dependent exponential hazard is used:

.. math::

    {\rm hazard} = \exp(a_i+b(t-t_{\rm HSV2-infected})+c M_{\rm i} + d H_{\rm i} + e_1 b_{\rm 0j} + e_2 b_{\rm 2j})

The value of :math:`a_i` can be set using ``person.hsv2.a.dist.type`` from the
:ref:`HSV2 related person settings <personhsv2opts>`. This value is taken from
the person that's already infected. The :math:`b` value can be configured using
``hsv2transmission.hazard.b``, and :math:`t_{\rm HSV2-infected}` is the time at which
the infected person acquired the HSV2 infection. :math:`M_{\rm i}` represents the gender effect and is taken from the person that's already HSV2-infected. It's value is 1 for male and 0 for female. The value :math:`H_{\rm i}` is an indicator for the HSV2-infected person being HIV-infected. It's value is 1 for HIV-infected and 0 for HIV-uninfected. The values of :math:`b_{\rm 0j}` and :math:`b_{\rm 2j}` specify the susceptibility of the uninfected person to both diseases and HSV2 only respectively. The value of :math:`b_{\rm 0j}` can be set using ``person.hiv.b0.dist.type`` from the :ref:`HIV related person settings <personhsv2opts>`. The value of :math:`b_{\rm 2j}` can be set using ``person.hsv2.b2.dist.type`` from the :ref:`HSV2 related person settings <personhsv2opts>`.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``hsv2transmission.hazard.b`` (0): |br|
   This configures the value of :math:`b` in the hazard above.
 - ``hsv2transmission.hazard.c`` (0): |br|
   This configures the value of c for the gender effect in the hazard above.
 - ``hsv2transmission.hazard.d`` (0): |br|
   This configures the value of d for the HIV effect in the hazard above.
 - ``hsv2transmission.hazard.e1`` (0): |br|
   This refers to the value of :math:`e_1` in the expression for the hazard. Set this parameter equal to 1 if you want to include the influence of susceptibility to both infections.
 - ``hsv2transmission.hazard.e2`` (0): |br|
   This refers to the value of :math:`e_2` in the expression for the hazard. Set this parameter equal to 1 if you want to include the influence of susceptibility to HSV2 only.
 - ``hsv2transmission.hazard.t_max`` (200): |br|
   As explained in the section about :ref:`'time limited' hazards <timelimited>`, an
   exponential function needs some kind of threshold value (after which it stays
   constant) to be able to perform the necessary calculations. This configuration
   value is a measure of this threshold.




