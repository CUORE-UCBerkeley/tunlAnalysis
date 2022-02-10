RUNS="89,90"
MATERIAL="MoPowder"
ENERGY="4"
./calibrate --material $MATERIAL --energy $ENERGY --runs $RUNS
./analyze --material $MATERIAL --energy $ENERGY --runs $RUNS