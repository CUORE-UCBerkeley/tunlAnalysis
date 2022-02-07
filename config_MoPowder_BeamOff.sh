RUNS="68,72,76,85"
MATERIAL="MoPowder"
ENERGY="0"

./calibrate --material $MATERIAL --energy $ENERGY -r $RUNS
./analyze --material $MATERIAL --energy $ENERGY -r $RUNS

