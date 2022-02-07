RUNS="91"
MATERIAL="EmptyBeam"
ENERGY="4"

./calibrate --material $MATERIAL --energy $ENERGY -r $RUNS
./analyze --material $MATERIAL --energy $ENERGY -r $RUNS