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
                [ "person.geo.dist2d", "distTypes2D", [
                                "discrete",
                                    [
                                        [ "densfile", "${SIMPACT_DATA_DIR}SWZ10adjv4.tif" ],
                                        [ "maskfile", "${SIMPACT_DATA_DIR}hhohho_mask.tiff" ],
                                        [ "width", 134.654 ],
                                        [ "height", 177.997 ],
                                        [ "flipy", "no" ]
                                ]
                        ]
                ]
            ],
            "info": [ 
                "The distribution specified by 'person.geo.dist2d' is used to assign (x,y)",
                "location coordinates to a person."
            ]
        },
		"PersonHealthSeekingPropensityDist": {
			"depends": null,
			"params": [
				[ "person.healthseekingpropensity.dist", "distTypes", [ "fixed", [ [ "value", 0 ]  ] ] ]
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

		})JSON");

