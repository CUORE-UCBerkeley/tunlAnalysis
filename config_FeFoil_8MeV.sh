RUNS="87"
MATERIAL="FeFoil"
ENERGY="8"

./calibrate --material $MATERIAL --energy $ENERGY --runs $RUNS
./analyze --material $MATERIAL --energy $ENERGY --runs $RUNS

