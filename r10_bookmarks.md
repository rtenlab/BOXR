## log_file

## timewarp latency breakdown:
plugins/timewarp_gl/plugin.cpp line 846 latency_rtd, latency_mtd ...

## IMU integration control
plugins/gtsam_integrator/plugin.cpp line 204 //// Need to integrate over a sliding window of 2 imu_type values.

# Make sure to set the following before running exp on Xavier
* jtop: set the power mode from the GUI or here
* jtop: jetson_clocks has to run to fix frequency (6 ctrl in jtop)

# atw_vio_pressure

Control Knobs for ATW:
* plugins/timewarp_gl/plugin.cpp: change the line inside warp to change the delayed execution time (== pressure time)
Control Knobs for VIO:
* Check the https://github.com/izenderi/open_vins_illixr/commits/atw_vio_pressure/ for the SHA of different ms delayed
* repalce cmake/GetOpenVINS.cmake SHA with the one from the above link
Control Knobs for RS (gldemo):
* plugins/gldemo/plugin.cpp: change the line inside warp to change the delayed execution time (== pressure time)
Control Knobs for IMUi (gtsam_integrator):
* plugins/gtsam_integrator/plugin.cpp: change the line inside warp to change the delayed execution time (== pressure time)