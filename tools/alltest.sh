set -e
rm -rf ptest/logs
rm -rf .pio
time python -m ptest runsim --clean -c ptest/configs/hootl_speedup.json -t EmptyCase -ni
time python -m ptest runsim --clean -c ptest/configs/hootl_speedup.json -t EmptySimCase -ni
time python -m ptest runsim --clean -c ptest/configs/hootl_speedup.json -t GomspaceCheckoutCase -ni
time python -m ptest runsim --clean -c ptest/configs/hootl_speedup.json -t DCDCCheckoutCase -ni
time python -m ptest runsim --clean -c ptest/configs/hootl_speedup.json -t PropStateMachineCase -ni
time python -m ptest runsim --clean -c ptest/configs/hootl_speedup.json -t SpinMotorsCase -ni
time python -m ptest runsim --clean -c ptest/configs/hootl_speedup.json -t ADCSCheckoutCase -ni
time python -m ptest runsim --clean -c ptest/configs/hootl_speedup.json -t PiksiCheckoutCase -ni
time python -m ptest runsim --clean -c ptest/configs/hootl_speedup.json -t DeploymentToInitHold -ni
time python -m ptest runsim --clean -c ptest/configs/hootl_speedup.json -t CICase -ni
time python -m ptest runsim --clean -c ptest/configs/hootl_speedup.json -t QuakeFaultHandler_Fast -ni
time python -m ptest runsim --clean -c ptest/configs/hootl_speedup.json -t PiksiFaultHandler_Fast -ni
time python -m ptest runsim --clean -c ptest/configs/hootl_speedup.json -t ADCSWheelFaultHandler -ni
time python -m ptest runsim --clean -c ptest/configs/hootl_speedup.json -t LowBattFaultHandler -ni
