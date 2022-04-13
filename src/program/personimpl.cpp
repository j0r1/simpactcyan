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
		"PersonBehaviorDist": {
			"depends": null,
			"params": [
				[ "person.behavior.dist2d", "distTypes2D", [ "fixed", [ [ "xvalue", 0 ], ["yvalue", 0]   ] ] ]
			], 
			"info": [
				"The distribution used to assing a health-seeking propensity to a person."
			]
		})JSON");


