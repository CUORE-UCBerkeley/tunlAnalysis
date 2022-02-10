RUNS="91"
MATERIAL="EmptyBeam"
ENERGY="4"
#CALIBFILE="calibration_MoPowder_4MeV"

./calibrate --material $MATERIAL --energy $ENERGY -runs $RUNS
./analyze --material $MATERIAL --energy $ENERGY -runs $RUNS 
#./analyze --material $MATERIAL --energy $ENERGY -runs $RUNS --fCalib $CALIBFILE