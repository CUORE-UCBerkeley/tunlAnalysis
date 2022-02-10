RUNS="85"
MATERIAL="MoPowder_Run85"
ENERGY="0"

./calibrate --material $MATERIAL --energy $ENERGY --runs $RUNS
./analyze --material $MATERIAL --energy $ENERGY --runs $RUNS

