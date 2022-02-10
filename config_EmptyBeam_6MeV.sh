RUNS="79"
MATERIAL="EmptyBeam"
ENERGY="6"

./calibrate --material $MATERIAL --energy $ENERGY --runs $RUNS
./analyze --material $MATERIAL --energy $ENERGY --runs $RUNS
#./analyze --material $MATERIAL --energy $ENERGY -runs $RUNS --fCalib $CALIBFILE