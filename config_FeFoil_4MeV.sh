RUNS="88"
MATERIAL="FeFoil"
ENERGY="4"

./calibrate --material $MATERIAL --energy $ENERGY --r $RUNS
./analyze --material $MATERIAL --energy $ENERGY --r $RUNS

