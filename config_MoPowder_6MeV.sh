RUNS="69,70,71,73,74,75"
MATERIAL="MoPowder"
ENERGY="6"
./calibrate --material $MATERIAL --energy $ENERGY -r $RUNS
./analyze --material $MATERIAL --energy 6 -r $RUNS