.. This is just a definition of |br| to be able to force a line break somewhere

.. |br| raw:: html

   <br/>
.. _maxart:

.. image:: _static/MaxART-Simpact.jpg
    :align: right
    :width: 15%

MaxART simulation
=================

Introduction
------------

The :ref:`Simpact Cyan <simpactcyan>` simulation framework also has extensions for simulations in
the context of the MaxART study. For the most part, the usage and settings are
the same as in the main Simpact Cyan program, and the reader is therefore
strongly suggested to consult the core Simpact Cyan :ref:`reference documentation <simpactcyan>`.
This document will only contain information regarding to settings that deviate
from the ones specified in the core documentation.

Notes regarding the installation are also the same as in the core documentation.
The necessary MaxART-specific software is automatically included when the
relevant :ref:`Simpact Cyan program <programs>` is installed.

Selecting the MaxART-specific simulation
----------------------------------------

When :ref:`running from the command line <commandline>`, instead of
using the executables ``simpact-cyan-release`` or ``simpact-cyan-debug``, you should
use ``maxart-release`` or ``maxart-debug``.

It is however usually far more convenient to :ref:`run the simulation from R <startingfromR>`
or to :ref:`run from Python <startingfromPython>`. In case the
R environment is used, the user must specify that MaxART related simulations
are to be performed by setting the simulation prefix to ``maxart``::

    simpact.set.simulation("maxart")

When the Python method is used, the function ``setSimulationPrefix`` must be called.
For example, if the simulation object has the name ``simpact``, the call would be::

    simpact.setSimulationPrefix("maxart")

Output
------

In addition to the :ref:`output files <outputfiles>` of the general simulation, there
is one additional MaxART-specific output file possible, specified by the 
``maxart.outfile.logsteps`` option. This output file will contain a table with
the different :ref:`health care facilities <maxartfacilities>`, the time at which
a study step took place, and an indication of the stage that the facility was in
during a specific step. This can be one of:

 - ``C`` for the control stage
 - ``T`` for the transitioning stage
 - ``I`` for the intervention stage
 - ``Post`` for when the study has ended

An example can be seen in the following notebook: `maxart-facilitysteps.ipynb <_static/maxart-facilitysteps.ipynb>`_

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``maxart.outfile.logsteps`` ('${SIMPACT_OUTPUT_PREFIX}maxartsteplog.csv'): |br|
   The name of the CSV file that will contain information about which facility was
   in which stage of the study during the simulation.

Simulation details
------------------

The simulation proceeds in largely the same fashion as described in :ref:`the core documentation <generalflow>`,
but there are a few alterations for the MaxART context. The study is specific
to the Hhohho region of Swaziland, and the :ref:`person settings <maxartperson>` have been
set up so that the location of a person is based on the population density in
the Hhohho region. The :ref:`health care facilities <maxartfacilities>` can be described
by various settings, including their names and locations and their randomization
used in the MaxART trial. For testing purposes, it has been made straightforward
to avoid using the true randomization settings used in the study.

The start of the MaxART study in the simulation, can be specified using the
:ref:`start of study <maxartstartstudy>` event. Once the study has been started, the first
:ref:`step event <maxartstepstudy>` is scheduled to take place a specific amount of time
later. Until that time, all health care facilities are marked as being in the 'control', or
'standard of care' phase. When the first step event fires, two facilities [*] are marked as being in 
a 'transition period', and a new step event is scheduled with the same interval.
If available, the firing of a step event will also advance the facilities that
previously were in the 'transition period' to the 'treatment for all' phase.
When no more facilities are available, the :ref:`end of study <maxartendstudy>` event is
scheduled to take place a specific time interval later.

Deciding if an HIV infected person may be treated, is done in the :ref:`HIV infection monitoring <maxartmonitoring>`
event. This is again :ref:`similar <monitoring>` as in the core program,
but the CD4 threshold to decide if a person is eligible for treatment, can be set
differently for the various study stages. The default settings are counts of 350
unless in the transition period or treatment for all period, in which case there's
no specific threshold anymore.

[*] The number of facilities actually depends on the information in the randomization
file, but the default is two.

.. _maxartperson:

Person settings
^^^^^^^^^^^^^^^

The configurable person settings are the same as in the :ref:`main program <person>`,
except for the geographical location of a person. In this case, the defaults are
set up in such a way that a person's location is chosen based on the population
density of the Hhohho region of Swaziland. The following iPython notebook illustrates
this: `maxart-popdens.ipynb <_static/maxart-popdens.ipynb>`_

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``person.geo.dist2d.type`` ('discrete' with settings for the Hhohho region of Swaziland): |br|
   This :ref:`two dimensional distribution <prob2d>` is used to assign a geographic
   location to each person. The ``densfile`` parameter is set to ``SWZ10adjv4.tif``, which contains
   information about the population density in Swaziland. To limit the geographical distribution
   to the one from the Hhohho region, the mask file ``hhohho_mask.tiff`` is used.


.. _maxartfacilities:

Participating health care facilities
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The health care facilities that participate in the MaxART study are specified
in a CSV file (``facilities.geo.coords``) that lists the name, the longitude and
the latitude of each facility. Because the person coordinates use an X and Y
distance relative to some corner of a map of Swaziland, these geographic
coordinates of the facilities cannot be used directly. Instead, they will be
transformed to X and Y positions based on the ``facilities.geo.start.latitude``,
``facilities.geo.start.longitude`` and ``facilities.geo.start.corner`` settings.
If ``facilities.outfile.facilityxypos`` is specified, the resulting X and Y
values will be written to a CSV file, so that they can be compared to the
person locations. This is illustrated in the following iPython notebook:
`maxart-popdens.ipynb <_static/maxart-popdens.ipynb>`_

The order in which the facilities are used in the study, is specified in the
CSV file in the ``facilities.randomization`` setting. 

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``facilities.geo.coords`` ('maxart-facilities.csv' from the data directory): |br|
   This is the name of the CSV file that specifies the names of the facilities
   in the study, together with their GPS coordinates. These coordinates must
   be transformed to X and Y values so that they can be related to the location
   of each person, and the values needed for this transformation are specified
   in the following three options.
 - ``facilities.geo.start.latitude`` (-25.7172): |br|
   Together with ``facilities.geo.start.longitude``, this specifies the origin
   of the X-Y coordinate system that should be used to relate the facility locations
   to the person locations.
 - ``facilities.geo.start.longitude`` (30.7901): |br|
   Together with ``facilities.geo.start.latitude``, this specifies the origin
   of the X-Y coordinate system that should be used to relate the facility locations
   to the person locations.
 - ``facilities.geo.start.corner`` ('top'): |br|
   This value can be "top" or "bottom", and specifies if Y distances should
   be positive if the location of a facility is more south (for 'top') than the latitude
   in ``facilities.geo.start.latitude``, or when more north (for 'bottom').
 - ``facilities.outfile.facilityxypos`` (not written by default): |br|
   If specified, the coordinates resulting from the transformation above, will
   be writted to this CSV file.

.. _maxartrandomization:

 - ``facilities.randomization`` ('maxart-randomization.csv' from the data directory): |br|
   This specifies the randomization of the health care facilities to be used in the
   simulation. For testing purposes, some fake randomization files can be
   downloaded here:

    - `maxart-randomization-fake_1.csv <_static/maxart-randomization-fake_1.csv>`_
    - `maxart-randomization-fake_2.csv <_static/maxart-randomization-fake_2.csv>`_
    - `maxart-randomization-fake_3.csv <_static/maxart-randomization-fake_3.csv>`_
    - `maxart-randomization-fake_4.csv <_static/maxart-randomization-fake_4.csv>`_
    - `maxart-randomization-fake_5.csv <_static/maxart-randomization-fake_5.csv>`_


Events
^^^^^^

All of the events described in the :ref:`main documentation <events>` are still
available. Below, only the events that have been altered or added will be described.

.. _maxartmonitoring:

HIV infection monitoring event
""""""""""""""""""""""""""""""

When a person has been :ref:`diagnosed <diagnosis>` as being infected with HIV,
monitoring events are scheduled to follow up on the progress of the disease
by inspecting the :ref:`person's CD4 count <cd4count>`. If this CD4 count is
below a certain configurable threshold, the person will be
offered antiretroviral treatment. Depending on the person's 
:ref:`willingness to accept <artacceptthreshold>` treatment, treatment will
then be started.

The threshold can be set differently depending on the stage a facility
is in. Different values can be set before the study starts and after it ends
(``monitoring.cd4.threshold.prestudy`` and ``monitoring.cd4.threshold.poststudy``),
and during the MaxART study it can be set differently for the control stage,
the transition stage and the intervention stage (the treatment for all period)
(``monitoring.cd4.threshold.instudy.controlstage``, ``monitoring.cd4.threshold.instudy.transitionstage``
and ``monitoring.cd4.threshold.instudy.interventionstage``).

Note that it is currently assumed that a person will receive such monitoring
at the health care facility that is geographically the closest one. This is
illustrated in the following iPython notebook: `maxart-monitoringfacilities.ipynb <_static/maxart-monitoringfacilities.ipynb>`_

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

 - ``monitoring.cd4.threshold.prestudy`` (350): |br|
   This is the threshold value for a person's CD4 count, _before_ the :ref:`start of the study <maxartstartstudy>`: 
   if the count is below this value, treatment will be offered.
 - ``monitoring.cd4.threshold.poststudy`` (350): |br|
   This is the threshold value for a person's CD4 count, _after_ the :ref:`end of the study <maxartendstudy>`: 
   if the count is below this value, treatment will be offered.
 - ``monitoring.cd4.threshold.instudy.controlstage`` (350): |br|
   This is the threshold value for a person's CD4 count, during the MaxART study, when the
   person is at a facility in the control stage. If the count is below this value, treatment will be offered.
 - ``monitoring.cd4.threshold.instudy.transitionstage`` ('inf'): |br|
   This is the threshold value for a person's CD4 count, during the MaxART study, when the
   person is at a facility in the transition stage. If the count is below this value, treatment will be offered.
 - ``monitoring.cd4.threshold.instudy.interventionstage`` ('inf'): |br|
   This is the threshold value for a person's CD4 count, during the MaxART study, when the
   person is at a facility in the intervention stage. If the count is below this value, treatment will be offered.
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

.. _maxartstartstudy:

Start of study event
""""""""""""""""""""

To mark the start of the MaxART study in the simulation, this event can be triggered at
a specific time (``maxart.starttime``). This has an effect on the threshold that will be
used in the :ref:`HIV monitoring <maxartmonitoring>` event: until this start of study event has
been fired, the 'pre-study' threshold will be used.

When this event has been fired, the participating health care facilities are all marked as
being in the control stage. A :ref:`study step <maxartstepstudy>` event will be scheduled
a specific time later, to advance a number of facilities (depending on the :ref:`randomization <maxartrandomization>`)
to the transition stage.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``maxart.starttime`` (5): |br|
   This is the simulation time at which this event will be fired, indicating the start
   of the MaxART study. To disable this, set to a negative value.

.. _maxartstepstudy:

Time step within study
""""""""""""""""""""""

When this event fires for the first time, a number of health care facilities 
(depending on the :ref:`randomization <maxartrandomization>`) are advanced from the control stage 
to the transition stage. At this point, a new time step event is scheduled to take
place, which will advance these facilities to the intervention stage, and will
place other facilities (again depending on the randomization) in the transition
stage. When no more facilities can be placed in the transition stage, an 
:ref:`end of study <maxartendstudy>` event will be scheduled.

The interval between the start of the study, the first time step event and
subsequent time step events, and between the last time step event and the
end of study event, is configured using the ``maxart.stepinterval`` option.

The following iPython notebook shows these steps for one of the fake randomizations
provided in the :ref:`health care facilities <maxartfacilities>` section: 
`maxart-facilitysteps.ipynb <_static/maxart-facilitysteps.ipynb>`_

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``maxart.stepinterval`` (0.33333): |br|
   This is the interval that will be used in between the time steps of the study 
   (defaults to four months).
   This same interval will be used between the :ref:`start of the study <maxartstartstudy>` and the first
   step event, and between the last step event and :ref:`end of study event <maxartendstudy>`.

.. _maxartendstudy:

End of study event
""""""""""""""""""

When a :ref:`time step event <maxartstepstudy>` fires, and no more health care facilities can
be advanced from the control stage to the transition stage, this indicates the the
study is almost done, and an end of study event will be scheduled. This event
will fire the same amount of time later as the interval between the steps.

There are no configurable options for this event, it only serves to mark the end
of the MaxART study in the simulation. Once the study is done, the 'post-study'
CD4 threshold will be used as configured in the :ref:`HIV monitoring <maxartmonitoring>`
event.


