# TUNLAnalysis
Scripts for TUNL 100Mo neutron activation analysis.

There are two `c++` scripts (i) to calibrate, and (ii) to reduce root files -- from mvme -- to higher level analysis. You will need `ROOT` on your system to compile and run the scripts. Currently, the scripts are setup to work on files that are a collection of (Material, NeutronEnergy) run pairs. 

Make sure that you create a folder before calling any of the script
``` $ mkdir ../processedFiles ```

This is where the calibration and other root files will be saved.

# Calibration

Currently, the calibration is done for all 4 detectors on the cumulative raw amplitude spectra for the collection of runs. The calibration, for now, is done only on 511-kev, 1460-keV, and 2615-keV, only. If you want to add more lines for calibration, you should edit, compile, and run the `Calibrate.cpp` file. 

``` g++ Calibrate.cpp `root-config --glibs --cflags` -lSpectrum -o calibrate ``` \\

```./calibrate --material MoPowder --energy 6```

Note that not all material energy combinations are possible. Please refer to spreadsheet to make sure you know what files you are processing. You can also make your own list with the right run numbers. 

# Analyze 

This script produces a root files which contains a tree with `energy` and `tof` parameters of the four Germanium detector. It also contains some `energy` and `tof` histograms for the corresponding channels with `tof` cuts. The `tof_corrected` histogram has been normalized to the range of `tof_prompt` cut. To compile and run do 

``` g++ Analyze.cpp `root-config --glibs --cflags` -lSpectrum -o analyze ``` \\

```./analyze --material MoPowder --energy 6```


