.. This is just a definition of |br| to be able to force a line break somewhere

.. |br| raw:: html

   <br/>

.. _outputfiles:

Output
======

By default, several log files are used, but they can be disabled by assigning an
empty string to the configuration property. If you're using the R or Python
interface, the full paths of these log files will be stored in the object returned
by ``simpact.run`` or the ``PySimpactCyan`` method ``run``.

Here is an overview of the relevant configuration options, their defaults (between
parentheses), and their meaning:

 - ``logsystem.outfile.logevents`` ('${SIMPACT_OUTPUT_PREFIX}eventlog.csv'): |br|
   Here, all events that take place are logged. See the section about the
   :ref:`configuration file <configfile>` for additional information regarding
   the ``SIMPACT_OUTPUT_PREFIX`` variable.
 - ``logsystem.outfile.logpersons`` ('${SIMPACT_OUTPUT_PREFIX}personlog.csv'): |br|
   In this file, information about every person in the simulation is stored.
   See the section about the
   :ref:`configuration file <configfile>` for additional information regarding
   the ``SIMPACT_OUTPUT_PREFIX`` variable.
 - ``logsystem.outfile.logrelations`` ('${SIMPACT_OUTPUT_PREFIX}relationlog.csv'): |br|
   Here, all relationships are logged. See the section about the
   :ref:`configuration file <configfile>` for additional information regarding
   the ``SIMPACT_OUTPUT_PREFIX`` variable.
 - ``logsystem.outfile.logtreatments`` ('${SIMPACT_OUTPUT_PREFIX}treatmentlog.csv'): |br|
   This file records information regarding treatments. See the section about the
   :ref:`configuration file <configfile>` for additional information regarding
   the ``SIMPACT_OUTPUT_PREFIX`` variable.
 - ``logsystem.outfile.logsettings`` ('${SIMPACT_OUTPUT_PREFIX}settingslog.csv'): |br|
   This file records the settings that were used at the start of the program
   and after each :ref:`simulation intervention <simulationintervention>`.
 - ``logsystem.outfile.loglocation`` ('${SIMPACT_OUTPUT_PREFIX}locationlog.csv'): |br|
   This file records the geographical locations that were assigned to a person.
   In case a non-trivial :ref:`geographical distribution <geodist>` is used and
   :ref:`relocations <relocation>` are enabled, this allows persons to be tracked
   throughout the simulation.

Event log
^^^^^^^^^

The event log is a CSV-like file, in which each line contains at least ten
entries:

  1. The simulation time at which the event took place
  2. A short description of the event
  3. The name of the first person involved in the event, or ``(none)``
  4. The person ID of the first person involved, or -1
  5. The gender (0 for a man, 1 for a woman) of the first person involved
     in the event, or -1
  6. The age of the first person involved in the event, or -1
  7. The name of the second person involved in the event, or ``(none)``
  8. The person ID of the second person involved, or -1
  9. The gender (0 for a man, 1 for a woman) of the second person involved
     in the event, or -1
  10. The age of the second person involved in the event, or -1

On a specific line, more entries may be present. In that case, the number of extra
entries will be a multiple of two, with the first entry of a pair being a short
description and the second the actual value. 

Some event descriptions are within
parentheses, like ``(childborn)`` or ``(relationshipended)``. These aren't actual
events themselves, but a kind of pseudo-event: they are log entries for certain
actions that are triggered by a real mNRM-event. For example, a :ref:`birth event <birth>`
will trigger the ``(childborn)`` pseudo-event, to be able to have a log entry for
the new person that is introduced into the population. The ``(relationshipended)``
pseudo-event is triggered both by the :ref:`dissolution of a relationship <dissolution>`
and the death of a person, either by :ref:`AIDS related causes <aidsmortality>` or
due to a :ref:`'normal' mortality event <mortality>`.

.. _personlog:

Person log
^^^^^^^^^^

The person log file is a CSV file with entries for each person in the simulation,
both for persons who are deceased and who are still alive when the simulation
finished. At the moment, the following columns are defined:

  1. ``ID``: The ID of the person that this line is about.
  2. ``Gender``: The gender (0 for a man, 1 for a woman) of the person that this
     line is about.
  3. ``TOB``: The time of birth of this person.
  4. ``TOD``: The time of death of this person, or ``inf`` (infinity) if the
     person is still alive when the simulation ends.
  5. ``IDF``: The ID of the father of this person, or -1 if the person is part
     of the initial population created at the start of the simulation.
  6. ``IDM``: The ID of the mother of this person, or -1 if the person is part
     of the initial population created at the start of the simulation.
  7. ``TODebut``: The simulation time at which the person became sexually active.
     If this is zero, it means that the person was already old enough at the
     start of the simulation, otherwise it's the time at which the :ref:`debut event <debut>`
     for this person took place (or ``inf`` if debut did not take place yet).
  8. ``FormEag``: The value of the :ref:`formation eagerness <eagerness>` parameter for
     this person for forming heterosexual relationships, which can be used in 
     the :ref:`formation event <formation>`.
  9. ``FormEagMSM``: The value of the :ref:`formation eagerness <eagerness>` parameter for
     this person for MSM relationships, which can be used in the :ref:`MSM formation event <formationmsm>`.
  10. ``InfectTime``: The time at which this person became HIV-infected, or ``inf``
      if the person was not infected. Will be the time at which either an 
      :ref:`HIV seeding event <hivseeding>` took place, or at which a :ref:`transmission event <transmission>`
      took place.
  11. ``InfectOrigID``: The ID of the person that infected the current person, or -1
      if the current person was not infected or infected by a :ref:`seeding event <hivseeding>`.
  12. ``InfectType``: This will be -1 if the person was not infected, 0 if the person
      got infected due to a :ref:`seeding event <hivseeding>` and 1 if due to a 
      :ref:`transmission event <transmission>`.
  13. ``log10SPVL``: If infected, this contains the logarithm (base 10) of the set-point
      viral load of this person that was first chosen (so _not_ affected by treatment).
      If not infected, this will be ``-inf``.
  14. ``TreatTime``: The time at which this person last received treatment, or ``inf`` if
      no treatment was received.
  15. ``XCoord``: Each person is assigned :ref:`a geographic location <geodist>`, of which this
      is the x-coordinate. In case :ref:`relocations <relocation>` are possible, the value
      in this log file will be the last one in the simulation. For more detailed information
      the :ref:`location log file <locationlog>` can be used.
  16. ``YCoord``: Each person is assigned :ref:`a geographic location <geodist>`, of which this
      is the y-coordinate. In case :ref:`relocations <relocation>` are possible, the value
      in this log file will be the last one in the simulation. For more detailed information
      the :ref:`location log file <locationlog>` can be used.
  17. ``AIDSDeath``: Indicates what the cause of death for this person was. Is -1 in case
      the person is still alive at the end of the simulation, 0 if the person died from
      non-AIDS related causes, and +1 in case the person's death was caused by AIDS.
  18. ``HSV2InfectTime``: This is the time at which this person became HSV2 infected, or
      ``inf`` in case the person is not infected.
  19. ``HSV2InfectOriginID``: The ID of the person that's the origin of the HSV2 infection, or -1
      if there is none (no infection or seeded).
  20. ``CD4atInfection``: As explained in :ref:`the CD4 count <cd4count>` related
      documentation, the CD4 values at start of infection and at time of death are currently
      chosen from a distribution. This column will contain the first of these values when
      the person is infected, or -1 otherwise.
  21. ``CD4atDeath``: Similar to the previous column, but will contain the CD4 value at the
      time of (AIDS related) death.

Relationship log
^^^^^^^^^^^^^^^^

In the relationship log, information about all dissolved relationships is logged, as
well as information about relationships that still exist when the simulation ends. The
file is a CSV file, currently with five columns:

  1. ``ID1``: The ID of the first person in the relationship.
  2. ``ID2``: The ID of the second person in the relationship.
  3. ``FormTime``: The time the relationship between these two people got formed.
  4. ``DisTime``: The time at which the relationship between these two people dissolved,
     or ``inf`` (infinity) if the relationship still existed when the simulation ended.
  5. ``AgeGap``: the age difference between the man and the woman in the relationship.
     A positive value means that the man is older than the woman.
  6. ``MSM``: If 1, then the relationship is an MSM relation, if 0 it's a heterosexual
     relationship.

Treatment log
^^^^^^^^^^^^^

This CSV file contains information about all antiretroviral treatments that took place 
during the simulation, both for treatments that are ongoing when the simulation ended
and for treatments that were ended during the simulation (due to the person :ref:`dropping out <dropout>`
or dying). The file currently has five columns:

  1. ``ID``: the ID of the person that received treatment
  2. ``Gender``: The gender (0 for a man, 1 for a woman) of the person that got treated
  3. ``TStart``: The time at which the treatment started
  4. ``TEnd``: The time at which the treatment ended (by dropping out or because the person
     died). In case treatment was still going on when the simulation ended, this is
     ``inf`` (infinity).
  5. ``DiedNow``: If the treatment got stopped because the person died, this flag will be 1.
     Otherwise it will be 0.
  6. ``CD4atARTstart``: The value of the CD4 count of this person right before the treatment
     started.

Settings log
^^^^^^^^^^^^

The settings log file contains the settings used throughout the simulation. The names of
the columns describe the configuration options being logged, and there will be a row with
values of these configuration options each time the settings got changed. The time at which
they were applied is also recorded in the log file in the first column.
The first row in the log file will describe the names of the columns and at least one
other row will be present, describing the settings used when the simulation was initialized.
In case :ref:`simulation intervention events <simulationintervention>` are used, additional rows
will be present.

This means that the structure of the settings log will look like the one below in case a
simulation intervention event was used to change a parameter of the :ref:`'agegap' formation hazard <agegaphazard>`
after ten years in the simulation::

    "t","aidsstage.final", ..., "formation.hazard.agegap.baseline", ...
     0 ,             0.5 , ...,                               0.1 , ...
    10 ,             0.5 , ...,                               0.2 , ...


.. _locationlog:

Location log
^^^^^^^^^^^^

The :ref:`person log file <personlog>` records the :ref:`geographical location <geodist>` of a
person, but this is only the last known location. By default, the location of a
person is set to (0, 0), but if a non-trivial geographical distribution is used instead,
:ref:`relocation events <relocation>` may be of interest. In this case however, a person
can have multiple locations throughout the simulation, and a single entry in the
:ref:`person log file <personlog>` will no longer suffice.

For this reason, each time a person is assigned a 2D location, an entry is written to
a location log file. The columns in this file are:

 1. ``Time``: the time at which the person was assigned the specified location.
 2. ``ID``: the identifier of the person this location applies to.
 3. ``XCoord``: the x-coordinate of the location of the person.
 4. ``YCoord``: the y-coordinate of the location of the person.

