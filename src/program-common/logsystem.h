#ifndef LOGSYSTEM_H

#define LOGSYSTEM_H

#include "logfile.h"

class ConfigSettings;
class ConfigWriter;
class GslRandomNumberGenerator;

class LogSystem
{
public: 
	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static LogFile logEvents, logPersons, logRelations, logTreatment, logSettings, logLocation, logViralLoadHIV, logGonorrhea, logGonorrheaTreat, logChlamydia,
	              logChlamydiaTreat, logSyphilis, logSyphilisTreat, logSyphilisStage, logPrep;
};

#define LogEvent LogSystem::logEvents
#define LogPerson LogSystem::logPersons
#define LogRelation LogSystem::logRelations
#define LogTreatment LogSystem::logTreatment
#define LogSettings LogSystem::logSettings
#define LogLocation LogSystem::logLocation
#define LogViralLoadHIV LogSystem::logViralLoadHIV
#define LogGonorrhea LogSystem::logGonorrhea
#define LogGonorrheaTreat LogSystem::logGonorrheaTreat
#define LogChlamydia LogSystem::logChlamydia
#define LogChlamydiaTreat LogSystem::logChlamydiaTreat
#define LogSyphilis LogSystem::logSyphilis
#define LogSyphilisTreat LogSystem::logSyphilisTreat
#define LogSyphilisStage LogSystem::logSyphilisStage
#define LogPrep LogSystem::logPrep

#endif // LOGSYSTEM_H
