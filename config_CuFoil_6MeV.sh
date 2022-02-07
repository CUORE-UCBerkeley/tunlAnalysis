RUNS="80"
MATERIAL="CuFoil"
ENERGY="6"

./calibrate --material $MATERIAL --energy $ENERGY -r $RUNS
./analyze --material $MATERIAL --energy $ENERGY -r $RUNS

