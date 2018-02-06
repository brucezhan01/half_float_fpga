#   Buils default project, do not run synthesis
#     vivado_hls -f run-hls.tcl 
#     vivado_hls -p prj_hls_darknet &

array set opt {
  doCsim      0
  doRTLsynth  1
  doRTLsim    0
  doExport    0
}

foreach arg $::argv {
  #puts "ARG $arg"
  foreach o [lsort [array names opt]] {
    if {[regexp "$o +(\\w+)" $arg unused opt($o)]} {
      puts "  Setting CONFIG  $o  [set opt($o)]"
    }
  }
}

puts "Final CONFIG"

set proj_dir "prj_hls_half_float"
open_project $proj_dir -reset
#set_top Kernel_0
set CFLAGS_K "-I../src -Wno-write-strings -std=c++0x -Wall -Wfatal-errors  -Ofast"
set CFLAGS_H "-I../src -Wno-write-strings -std=c++0x -Wall -Wfatal-errors  -Ofast"
add_files ../src/kernel.cpp -cflags "$CFLAGS_K"
add_files  -tb ../src/main.cpp    -cflags "$CFLAGS_H"

open_solution "solution1"
config_compile -ignore_long_run_time
#config_schedule -effort medium -verbose

set_part {xcku115-flvb2104-2-e} -tool vivado

create_clock -period 3.333333 -name default

set run_args ""

if {$opt(doCsim)} {
  puts "***** C SIMULATION *****"
  csim_design -ldflags "-lz -lrt -lstdc++" -argv "$run_args"
}

if {$opt(doRTLsynth)} {
  puts "***** C/RTL SYNTHESIS *****"
  csynth_design
  if {$opt(doRTLsim)} {
    puts "***** C/RTL SIMULATION *****"
    cosim_design -trace_level all -ldflags "-lrt" -argv "$run_args"
  }
}

if {$opt(doExport)} {
    export_design -evaluate verilog -format ipxact
}

quit
