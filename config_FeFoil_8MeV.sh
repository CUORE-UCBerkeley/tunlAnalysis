RUNS="87"
MATERIAL="FeFoil"
ENERGY="8"

./calibrate --material $MATERIAL --energy $ENERGY -r $RUNS
./analyze --material $MATERIAL --energy $ENERGY -r $RUNS

