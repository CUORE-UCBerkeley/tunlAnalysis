RUNS="58,59,60,61,62,63,64,65,66"
MATERIAL="MoFoil"
ENERGY="6"

./calibrate --material $MATERIAL --energy $ENERGY -r $RUNS
./analyze --material $MATERIAL --energy $ENERGY -r $RUNS

