#!/bin/bash

# Base directory for experiments
base_dir="fps_exp"

current_time=$(date +"%Y%m%d-%H%M%S")
log_dir="${base_dir}/${current_time}"
mkdir -p "$log_dir"

rm -rf demo_data
cp -r /home/ziliang/Desktop/demo_data_material ./demo_data
app="material"

for i in 10 20 30 40 50 60 70 80 90 100 110 120 130 140
do
	echo "Starting to build $i FPS..."
	file="./${base_dir}/_${i}.hpp"
	cp $file ./include/illixr/global_module_defs.hpp
	echo "file passed: $file"
	bash rebuild.sh
	echo "Finished $i FPS build!"

	echo "Exp starts in 10 seconds"
	sleep 10

	# TODO1: Change the --data and --demo_data to the correct path in V102
	logfile="${log_dir}/${app}_V102_${i}fps.log"
	echo "Starting exp: $i fps..."
	# Special Variable SECONDS
	SECONDS=0
	main.dbg.exe --plugins=offline_imu,offline_cam,gtsam_integrator,pose_prediction,gldemo,timewarp_gl --vis=openvins --data=/home/ziliang/data/V1_02/mav0 --demo_data=/home/ziliang/Developer/Git/ILLIXR_sync/demo_data --enable_offload=false --enable_alignment=false --enable_verbose_errors=false --enable_pre_sleep=false > "$logfile" 2>&1
	duration=$SECONDS
	echo "exp $i fps Finished in $duration seconds"

	sleep 5

	# TODO2: Change the --data and --demo_data to the correct path in MH01
	logfile="${log_dir}/${app}_MH01_${i}fps.log"
	echo "Starting exp: $i fps..."
	# Special Variable SECONDS
	SECONDS=0
	main.dbg.exe --plugins=offline_imu,offline_cam,gtsam_integrator,pose_prediction,gldemo,timewarp_gl --vis=openvins --data=/home/ziliang/data/MH_01/mav0 --demo_data=/home/ziliang/Developer/Git/ILLIXR_sync/demo_data --enable_offload=false --enable_alignment=false --enable_verbose_errors=false --enable_pre_sleep=false > "$logfile" 2>&1
	duration=$SECONDS
	echo "exp $i fps Finished in $duration seconds"
done

rm -rf demo_data
cp -r /home/ziliang/Desktop/demo_data_platformer ./demo_data
app="platformer"

for i in 10 20 30 40 50 60 70 80 90 100 110 120 130 140
do
	echo "Starting to build $i FPS..."
	file="./${base_dir}/_${i}.hpp"
	cp $file ./include/illixr/global_module_defs.hpp
	echo "file passed: $file"
	bash rebuild.sh
	echo "Finished $i FPS build!"

	echo "Exp starts in 10 seconds"
	sleep 10

	# TODO1: Change the --data and --demo_data to the correct path in V102
	logfile="${log_dir}/${app}_V102_${i}fps.log"
	echo "Starting exp: $i fps..."
	# Special Variable SECONDS
	SECONDS=0
	main.dbg.exe --plugins=offline_imu,offline_cam,gtsam_integrator,pose_prediction,gldemo,timewarp_gl --vis=openvins --data=/home/ziliang/data/V1_02/mav0 --demo_data=/home/ziliang/Developer/Git/ILLIXR_sync/demo_data --enable_offload=false --enable_alignment=false --enable_verbose_errors=false --enable_pre_sleep=false > "$logfile" 2>&1
	duration=$SECONDS
	echo "exp $i fps Finished in $duration seconds"

	sleep 5

	# TODO2: Change the --data and --demo_data to the correct path in MH01
	logfile="${log_dir}/${app}_MH01_${i}fps.log"
	echo "Starting exp: $i fps..."
	# Special Variable SECONDS
	SECONDS=0
	main.dbg.exe --plugins=offline_imu,offline_cam,gtsam_integrator,pose_prediction,gldemo,timewarp_gl --vis=openvins --data=/home/ziliang/data/MH_01/mav0 --demo_data=/home/ziliang/Developer/Git/ILLIXR_sync/demo_data --enable_offload=false --enable_alignment=false --enable_verbose_errors=false --enable_pre_sleep=false > "$logfile" 2>&1
	duration=$SECONDS
	echo "exp $i fps Finished in $duration seconds"
done