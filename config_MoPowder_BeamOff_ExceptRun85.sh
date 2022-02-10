RUNS="68,72,76"
MATERIAL="MoPowder_ExceptRun85"
ENERGY="0"

./calibrate --material $MATERIAL --energy $ENERGY --runs $RUNS
./analyze --material $MATERIAL --energy $ENERGY --runs $RUNS

