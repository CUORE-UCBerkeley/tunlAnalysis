RUNS="80"
MATERIAL="CuFoil"
ENERGY="6"
CALIBFILE="calibration_MoPowder_8MeV"

./calibrate --material $MATERIAL --energy $ENERGY --runs $RUNS  
./analyze --material $MATERIAL --energy $ENERGY --runs $RUNS --fCalib $CALIBFILE

