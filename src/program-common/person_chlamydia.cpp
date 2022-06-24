#include "person_chlamydia.h"


Person_Chlamydia::Person_Chlamydia(): Person_STI(), m_disease_state(Chlamydia_DiseaseState::Susceptible) {}

Person_Chlamydia::~Person_Chlamydia() {}

bool Person_Chlamydia::isInfected() const {
	return (m_disease_state == Chlamydia_DiseaseState::Exposed
			|| m_disease_state == Chlamydia_DiseaseState::SymptomaticInfectious
			|| m_disease_state == Chlamydia_DiseaseState::AsymtomaticInfectious);
}
