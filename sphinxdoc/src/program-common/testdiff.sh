#!/bin/bash

for s in {10..1000} ; do
	d=seed_${s}
	#echo "Seed: $s"
	mkdir $d
	(cd $d
	mkdir basic
	mkdir opt
	( cd basic ; MNRM_DEBUG_SEED=$s simpact-cyan-basic ../../defaultcfg.txt 0 >/dev/null 2>&1 )
	( cd opt ; MNRM_DEBUG_SEED=$s simpact-cyan-opt-debug ../../defaultcfg.txt 0 >/dev/null 2>&1 )
	X=`diff basic/eventlog.csv opt/eventlog.csv`
	if ! [ -z "$X" ] ; then
		echo -n "$s "
	fi
	)
done
echo
