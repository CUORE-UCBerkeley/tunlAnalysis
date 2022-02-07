RUNS="81,82,83,84,86"
MATERIAL="MoPowder"
ENERGY="8"
./calibrate --material $MATERIAL --energy $ENERGY -r $RUNS
./analyze --material $MATERIAL --energy $ENERGY -r $RUNS