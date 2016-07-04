#!/bin/bash

nodes=(50)
txpower=(1 10 100 1000)
routep=("AODV")
lossmodel=("FixedRSSDefault" "FixedRSSMod" "MatrixDefault" "MatrixMod" "Range" "RandomDefault" "RandomModified" "COSTHata" "Friis" "LogDistance" "ThreeLogDistance" "TwoRayGround" "TwoRayGroundMod" "JakesonFriis" "NakagamionFriis")

for rp in "${routep[@]}"
	do 	   
	for n in "${nodes[@]}"
		do
		for tp in "${txpower[@]}"
			do
			for lm in "${lossmodel[@]}"
				do
			 		./waf --run "scratch/p4 --routeProt=$rp --numNodes=$n --txPower=$tp --lossmodel=$lm" | tail -n 2
				done
			done	
		done
	done
exit 0
