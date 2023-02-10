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
        "PersonCondomUse": {
            "depends": null,
            "params": [
                [ "person.condomuse.dist", "distTypes", [ "fixed", [ [ "value", 0 ] ] ] ],
				[ "person.condomuse.concordancefactor", 1],
				[ "person.condomuse.artfactor", 1],
				[ "person.condomuse.prepfactor", 1]
            ],
            "info": [ 
                "The distribution specified by 'person.condomuse.dist' is used to assign",
                "a condom use probability discordant partnerships to a person. This can be adjusted",
				"by a factor for concordant relationships and a factor for ART as well."
            ]
        },
		"PersonSexualRoleDist": {
			"depends": null,
			"params": [
				[ "person.sexualrole.dist", "distTypes", [ "discrete.inline", [ [ "floor", "yes" ], [ "xvalues", "0, 1, 2, 3" ], [ "yvalues", "1, 0, 0, 0" ] ] ] ]
			],
			"info": [ "TODO" ]
		},
		"PersonTreatAcceptance": {
		  "depends": null,
		  "params": [
		  [ "person.sti.treat.accept.threshold.dist", "distTypes", ["fixed", [ ["value", 0.5 ] ] ] ]
		  ],
              "info": [
		  "This parameter specifies a distribution from which a number will be chosen",
		  "for each person, and which serves as the threshold to take STI treatment.",
		  "When eligible for treatment, a random number will be chosen uniformly from",
		  "[0,1], and treatment will only be started if this number is smaller than the",
		  "threshold. By default, everyone will just have a 50/50 chance of starting",
		  "treatment when possible. ",
		  "",
		  "If this distribution returns a low value (close to zero), it means that ",
		  "there's little chance of accepting treatment; if the value is higher (close to",
		  "one), treatment will almost always be accepted."
              ]
		})JSON");
