#include "logsystem.h"
#include "configsettings.h"
#include "configwriter.h"
#include "jsonconfig.h"
#include "configfunctions.h"

using namespace std;

void LogSystem::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	string eventLogFile, personLogFile, relationLogFile, treatmentLogFile, settingsLogFile;
	string locationLogFile, hivVLLogFile, gonorrheaLogFile, gonorrheatreatLogFile, chlamydiaLogFile, chlamydiatreatLogFile;
	string syphilisLogFile, syphilistreatLogFile, syphilisstageLogFile, hsv2LogFile, hsv2stageLogFile, prepLogFile;
	bool_t r;

	if (!(r = config.getKeyValue("logsystem.outfile.logevents", eventLogFile)) ||
	    !(r = config.getKeyValue("logsystem.outfile.logrelations", relationLogFile)) ||
	    !(r = config.getKeyValue("logsystem.outfile.logpersons", personLogFile)) ||
	    !(r = config.getKeyValue("logsystem.outfile.logtreatments", treatmentLogFile)) ||
		!(r = config.getKeyValue("logsystem.outfile.logsettings", settingsLogFile)) ||
		!(r = config.getKeyValue("logsystem.outfile.loglocation", locationLogFile)) ||
		!(r = config.getKeyValue("logsystem.outfile.logviralloadhiv", hivVLLogFile)) ||
		!(r = config.getKeyValue("logsystem.outfile.loggonorrhea", gonorrheaLogFile)) ||
		!(r = config.getKeyValue("logsystem.outfile.loggonorrheatreat", gonorrheatreatLogFile)) ||
		!(r = config.getKeyValue("logsystem.outfile.logchlamydia", chlamydiaLogFile)) ||
		!(r = config.getKeyValue("logsystem.outfile.logchlamydiatreat", chlamydiatreatLogFile)) ||
		!(r = config.getKeyValue("logsystem.outfile.logsyphilis", syphilisLogFile)) ||
		!(r = config.getKeyValue("logsystem.outfile.logsyphilistreat", syphilistreatLogFile)) ||
		!(r = config.getKeyValue("logsystem.outfile.logsyphilisstage", syphilisstageLogFile)) ||
		!(r = config.getKeyValue("logsystem.outfile.loghsv2", hsv2LogFile)) ||
		!(r = config.getKeyValue("logsystem.outfile.loghsv2stage", hsv2stageLogFile)) ||
		!(r = config.getKeyValue("logsystem.outfile.logprep", prepLogFile))
	    )
		abortWithMessage(r.getErrorString());

	if (eventLogFile.length() > 0)
	{
		if (!(r = logEvents.open(eventLogFile)))
			abortWithMessage("Unable to open event log file: " + r.getErrorString());
	}

	if (personLogFile.length() > 0)
	{
		if (!(r = logPersons.open(personLogFile)))
			abortWithMessage("Unable to open person log file: " + r.getErrorString());
	}

	if (relationLogFile.length() > 0)
	{
		if (!(r = logRelations.open(relationLogFile)))
			abortWithMessage("Unable to open relationship log file: " + r.getErrorString());
	}

	if (treatmentLogFile.length() > 0)
	{
		if (!(r = logTreatment.open(treatmentLogFile)))
			abortWithMessage("Unable to open treatment log file: " + r.getErrorString());
	}

	if (settingsLogFile.length() > 0)
	{
		if (!(r = logSettings.open(settingsLogFile)))
			abortWithMessage("Unable to open settings log file: " + r.getErrorString());
	}

	if (locationLogFile.length() > 0)
	{
		if (!(r = logLocation.open(locationLogFile)))
			abortWithMessage("Unable to open location log file: " + r.getErrorString());
	}

	if (hivVLLogFile.length() > 0)
	{
		if (!(r = logViralLoadHIV.open(hivVLLogFile)))
			abortWithMessage("Unable to open HIV viral load log file: " + r.getErrorString());
	}
	
	if (gonorrheaLogFile.length() > 0)
	{
	  if (!(r = logGonorrhea.open(gonorrheaLogFile)))
	    abortWithMessage("Unable to open Gonorrhea log file: " + r.getErrorString());
	}
	
	if (gonorrheatreatLogFile.length() > 0)
	{
	  if (!(r = logGonorrheaTreat.open(gonorrheatreatLogFile)))
	    abortWithMessage("Unable to open Gonorrhea treatment log file: " + r.getErrorString());
	}
	
	if (chlamydiaLogFile.length() > 0)
	{
	  if (!(r = logChlamydia.open(chlamydiaLogFile)))
	    abortWithMessage("Unable to open Chlamydia log file: " + r.getErrorString());
	}
	
	if (chlamydiatreatLogFile.length() > 0)
	{
	  if (!(r = logChlamydiaTreat.open(chlamydiatreatLogFile)))
	    abortWithMessage("Unable to open Chlamydia treatment log file: " + r.getErrorString());
	}
	
	if (syphilisLogFile.length() > 0)
	{
	  if (!(r = logSyphilis.open(syphilisLogFile)))
	    abortWithMessage("Unable to open Syphilis log file: " + r.getErrorString());
	}
	
	if (syphilistreatLogFile.length() > 0)
	{
	  if (!(r = logSyphilisTreat.open(syphilistreatLogFile)))
	    abortWithMessage("Unable to open Syphilis treatment log file: " + r.getErrorString());
	}
	
	if (syphilisstageLogFile.length() > 0)
	{
	  if (!(r = logSyphilisStage.open(syphilisstageLogFile)))
	    abortWithMessage("Unable to open Syphilis stage log file: " + r.getErrorString());
	}
	
	if (hsv2LogFile.length() > 0)
	{
	  if (!(r = logHSV2.open(hsv2LogFile)))
	    abortWithMessage("Unable to open HSV2 log file: " + r.getErrorString());
	}
	
	if (hsv2stageLogFile.length() > 0)
	{
	  if (!(r = logHSV2Stage.open(hsv2stageLogFile)))
	    abortWithMessage("Unable to open HSV2 stage log file: " + r.getErrorString());
	}
	
	if (prepLogFile.length() > 0)
	{
	  if (!(r = logPrep.open(prepLogFile)))
	    abortWithMessage("Unable to open PrEP log file: " + r.getErrorString());
	}
	
	

	logPersons.print("\"ID\",\"Gender\",\"TOB\",\"TOD\",\"IDF\",\"IDM\",\"TODebut\",\"FormEag\",\"FormEagMSM\",\"SexualRole\",\"InfectTime\",\"InfectOrigID\",\"InfectType\",\"log10SPVL\",\"TreatTime\",\"XCoord\",\"YCoord\",\"AIDSDeath\",\"HSV2InfectTime\",\"HSV2InfectOriginID\",\"CD4atInfection\",\"CD4atDeath\",\"HealthSeekingPropensity\",\"InfectSite\",\"NumSTIDiag12m\"");
	logRelations.print("\"ID1\",\"ID2\",\"FormTime\",\"DisTime\",\"AgeGap\",\"MSM\"");
	logTreatment.print("\"ID\",\"Gender\",\"TStart\",\"TEnd\",\"DiedNow\",\"CD4atDiagnosis\",\"CD4atARTstart\"");
	logLocation.print("\"Time\",\"ID\",\"XCoord\",\"YCoord\"");
	logViralLoadHIV.print("\"Time\",\"ID\",\"Desc\",\"Log10SPVL\",\"Log10VL\",\"CD4Count\"");
	logGonorrhea.print("\"ID\",\"InfectOrigID\",\"Gender\",\"SexualRole\",\"InfectTime\",\"InfectType\",\"InfectSite\",\"DiseaseStage\"");
	logGonorrheaTreat.print("\"ID\",\"Gender\",\"RecoveryTime\",\"Treated\"");
	logChlamydia.print("\"ID\",\"InfectOrigID\",\"Gender\",\"SexualRole\",\"InfectTime\",\"InfectType\",\"InfectSite\",\"DiseaseStage\"");
	// logChlamydiaTreat.print("\"ID\",\"Gender\",\"RecoveryTime\",\"Treated\",\"Diagnosed\"");
	logChlamydiaTreat.print("\"ID\",\"Gender\",\"RecoveryTime\",\"Treated\"");
	logSyphilis.print("\"ID\",\"InfectOrigID\",\"Gender\",\"SexualRole\",\"InfectTime\",\"InfectType\",\"InfectSite\",\"DiseaseStage\"");
	logSyphilisTreat.print("\"ID\",\"Gender\",\"RecoveryTime\",\"Treated\"");
	logSyphilisStage.print("\"Time\",\"ID\",\"Stage\"");
	logHSV2.print("\"ID\",\"InfectOrigID\",\"Gender\",\"SexualRole\",\"InfectTime\",\"InfectType\",\"InfectSite\",\"DiseaseStage\"");
	logHSV2Stage.print("\"Time\",\"ID\",\"Stage\"");
	logPrep.print("\"ID\",\"StartTime\",\"StopTime\",\"StopReason\"");
}

void LogSystem::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	if (!(r = config.addKey("logsystem.outfile.logevents", logEvents.getFileName())) ||
	    !(r = config.addKey("logsystem.outfile.logrelations", logRelations.getFileName())) ||
	    !(r = config.addKey("logsystem.outfile.logpersons", logPersons.getFileName())) ||
	    !(r = config.addKey("logsystem.outfile.logtreatments", logTreatment.getFileName())) ||
		!(r = config.addKey("logsystem.outfile.logsettings", logSettings.getFileName())) ||
		!(r = config.addKey("logsystem.outfile.loglocation", logLocation.getFileName())) ||
		!(r = config.addKey("logsystem.outfile.logviralloadhiv", logViralLoadHIV.getFileName())) ||
		!(r = config.addKey("logsystem.outfile.loggonorrhea", logGonorrhea.getFileName())) ||
		!(r = config.addKey("logsystem.outfile.loggonorrheatreat", logGonorrheaTreat.getFileName())) ||
		!(r = config.addKey("logsystem.outfile.logchlamydia", logChlamydia.getFileName())) ||
		!(r = config.addKey("logsystem.outfile.logchlamydiatreat", logChlamydiaTreat.getFileName())) ||
		!(r = config.addKey("logsystem.outfile.logsyphilis", logSyphilis.getFileName())) ||
		!(r = config.addKey("logsystem.outfile.logsyphilistreat", logSyphilisTreat.getFileName())) ||
		!(r = config.addKey("logsystem.outfile.logsyphilisstage", logSyphilisStage.getFileName())) ||
		!(r = config.addKey("logsystem.outfile.loghsv2", logHSV2.getFileName())) ||
		!(r = config.addKey("logsystem.outfile.loghsv2stage", logHSV2Stage.getFileName())) ||
		!(r = config.addKey("logsystem.outfile.logprep", logPrep.getFileName()))
	    )
		abortWithMessage(r.getErrorString());
}

LogFile LogSystem::logEvents;
LogFile LogSystem::logPersons;
LogFile LogSystem::logRelations;
LogFile LogSystem::logTreatment;
LogFile LogSystem::logSettings;
LogFile LogSystem::logLocation;
LogFile LogSystem::logViralLoadHIV;
LogFile LogSystem::logGonorrhea;
LogFile LogSystem::logGonorrheaTreat;
LogFile LogSystem::logChlamydia;
LogFile LogSystem::logChlamydiaTreat;
LogFile LogSystem::logSyphilis;
LogFile LogSystem::logSyphilisTreat;
LogFile LogSystem::logSyphilisStage;
LogFile LogSystem::logHSV2;
LogFile LogSystem::logHSV2Stage;
LogFile LogSystem::logPrep;


ConfigFunctions logSystemConfigFunctions(LogSystem::processConfig, LogSystem::obtainConfig, "00_LogSystem", "__first__");

JSONConfig logSystemJSONConfig(R"JSON(
        "LogSystem": { 
            "depends": null,
            "params": [ 
                ["logsystem.outfile.logevents", "${SIMPACT_OUTPUT_PREFIX}eventlog.csv" ],
                ["logsystem.outfile.logpersons", "${SIMPACT_OUTPUT_PREFIX}personlog.csv" ],
                ["logsystem.outfile.logrelations", "${SIMPACT_OUTPUT_PREFIX}relationlog.csv" ], 
                ["logsystem.outfile.logtreatments", "${SIMPACT_OUTPUT_PREFIX}treatmentlog.csv" ],
				["logsystem.outfile.logsettings", "${SIMPACT_OUTPUT_PREFIX}settingslog.csv" ],
				["logsystem.outfile.loglocation", "${SIMPACT_OUTPUT_PREFIX}locationlog.csv" ],
				["logsystem.outfile.logviralloadhiv", "${SIMPACT_OUTPUT_PREFIX}hivviralloadlog.csv" ],
    ["logsystem.outfile.loggonorrhea", "${SIMPACT_OUTPUT_PREFIX}gonorrhealog.csv" ],
    ["logsystem.outfile.loggonorrheatreat", "${SIMPACT_OUTPUT_PREFIX}gonorrheatreatlog.csv" ],
    ["logsystem.outfile.logchlamydia", "${SIMPACT_OUTPUT_PREFIX}chlamydialog.csv" ],
    ["logsystem.outfile.logchlamydiatreat", "${SIMPACT_OUTPUT_PREFIX}chlamydiatreatlog.csv" ],
    ["logsystem.outfile.logsyphilis", "${SIMPACT_OUTPUT_PREFIX}syphilislog.csv" ],
    ["logsystem.outfile.logsyphilistreat", "${SIMPACT_OUTPUT_PREFIX}syphilistreatlog.csv" ],
    ["logsystem.outfile.logsyphilisstage", "${SIMPACT_OUTPUT_PREFIX}syphilisstagelog.csv" ],
    ["logsystem.outfile.loghsv2", "${SIMPACT_OUTPUT_PREFIX}hsv2log.csv" ],
    ["logsystem.outfile.loghsv2stage", "${SIMPACT_OUTPUT_PREFIX}hsv2stagelog.csv" ],
    ["logsystem.outfile.logprep", "${SIMPACT_OUTPUT_PREFIX}preplog.csv" ]
                  ],
            "info": null                          
        })JSON");

