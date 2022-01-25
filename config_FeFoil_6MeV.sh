RUNS="67,77,78"
MATERIAL="FeFoil"
ENERGY="6"

./calibrate --material $MATERIAL --energy $ENERGY -r $RUNS
./analyze --material $MATERIAL --energy $ENERGY -r $RUNS

