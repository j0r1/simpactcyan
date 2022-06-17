#include "personimpl.h"
#include "person.h"
#include "jsonconfig.h"

PersonImpl::PersonImpl(Person &p) : m_person(p)
{
}

PersonImpl::~PersonImpl()
{
}

JSONConfig personImplJSONConfig(R"JSON(
        "PersonGeoDist": {
            "depends": null,
            "params": [
                [ "person.geo.dist2d", "distTypes2D" ]
            ],
            "info": [ 
                "The distribution specified by 'person.geo.dist2d' is used to assign (x,y)",
                "location coordinates to a person."
            ]
        },
		"PersonHealthSeekingPropensityDist": {
			"depends": null,
			"params": [
				[ "person.healthseekingpropensity.dist", "distTypes", [ "fixed", [ [ "value", 0 ] ] ] ]
			], 
			"info": [
				"The distribution used to assign a health-seeking propensity to a person."
			]
		},
        "PersonCondomUseDist": {
            "depends": null,
            "params": [
                [ "person.condomuse.dist2d", "distTypes2D" ]
            ],
            "info": [ 
                "The distribution specified by 'person.condomuse.dist2d' is used to assign",
                "a condom use probability for concordant (x) and discordant (y) partnerships to a person."
            ]
        })JSON");


