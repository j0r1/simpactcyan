set(SOURCES_SIMPACT
	../program/personimpl.cpp
	../program/start.cpp
	coarsemap.cpp
	person.cpp
	person_relations.cpp
	person_hiv.cpp
	person_hsv2.cpp
     logsystem.cpp
     simpactpopulation.cpp
	eventmortalitybase.cpp
	eventmortality.cpp
	eventaidsmortality.cpp
	eventformation.cpp
	eventdissolution.cpp
	eventdebut.cpp
	eventhivtransmission.cpp
	eventhsv2transmission.cpp
	eventchronicstage.cpp
	eventseedbase.cpp
	eventhivseed.cpp
	eventhsv2seed.cpp
	eventintervention.cpp
	eventaidsstage.cpp
	eventconception.cpp
	eventbirth.cpp
	eventdiagnosis.cpp
	eventmonitoring.cpp
	eventdropout.cpp
	eventperiodiclogging.cpp
	eventsyncpopstats.cpp
	eventsyncrefyear.cpp
	eventcheckstopalgorithm.cpp
	eventrelocation.cpp
	vspmodellogweibullwithnoise.cpp
	vspmodellogdist.cpp
	simpactevent.cpp
	evthazardformationsimple.cpp
	evthazardformationagegap.cpp
	evthazardformationagegaprefyear.cpp
	evthazarddissolution.cpp
	hazardfunctionformationsimple.cpp
	hazardfunctionformationagegap.cpp
	hazardfunctionformationagegaprefyear.cpp
	main_hazardtest.cpp
	main.cpp
	signalhandlers.cpp
	configutil.cpp
	aidstodutil.cpp
	configsettingslog.cpp
	)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/../program-common/")
add_simpact_executable(simpact-HSV2 ${SOURCES_SIMPACT})
install_simpact_executable(simpact-HSV2)


