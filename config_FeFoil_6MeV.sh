RUNS="67,77,78"
MATERIAL="FeFoil"
ENERGY="6"

./calibrate --material $MATERIAL --energy $ENERGY --runs $RUNS
./analyze --material $MATERIAL --energy $ENERGY --runs $RUNS

