RUNS="79"
MATERIAL="EmptyBeam"
ENERGY="6"

./calibrate --material $MATERIAL --energy $ENERGY -r $RUNS
./analyze --material $MATERIAL --energy $ENERGY -r $RUNS