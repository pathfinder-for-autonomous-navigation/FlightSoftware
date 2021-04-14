set -e
rm -rf ptest/logs
rm -rf .pio
time python -m ptest runsim --clean -c ptest/configs/ci.json -t EmptyCase -ni
time python -m ptest runsim --clean -c ptest/configs/ci.json -t EmptySimCase -ni
time python -m ptest runsim --clean -c ptest/configs/ci.json -t NothingCase -ni
time python -m ptest runsim --clean -c ptest/configs/ci.json -t GomspaceCheckoutCase -ni
time python -m ptest runsim --clean -c ptest/configs/ci.json -t DCDCCheckoutCase -ni
time python -m ptest runsim --clean -c ptest/configs/ci.json -t PropStateMachineCase -ni
time python -m ptest runsim --clean -c ptest/configs/ci.json -t SpinMotorsCase -ni
time python -m ptest runsim --clean -c ptest/configs/ci.json -t ADCSCheckoutCase -ni
time python -m ptest runsim --clean -c ptest/configs/ci.json -t PiksiCheckoutCase -ni
time python -m ptest runsim --clean -c ptest/configs/ci.json -t DeploymentToInitHold -ni
time python -m ptest runsim --clean -c ptest/configs/ci.json -t CICase -ni
time python -m ptest runsim --clean -c ptest/configs/ci.json -t QuakeFaultHandler_Fast -ni
time python -m ptest runsim --clean -c ptest/configs/ci.json -t PiksiFaultHandler_Fast -ni
time python -m ptest runsim --clean -c ptest/configs/ci.json -t ADCSWheelFaultHandler -ni
time python -m ptest runsim --clean -c ptest/configs/ci.json -t LowBattFaultHandler -ni
