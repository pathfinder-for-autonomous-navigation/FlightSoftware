#!/bin/sh

# This script is a work in progress

# This should only be run on Mac!
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "This script should only be run on MacOS."
    exit 1
fi

# A PlatformIO installation is required for this script
if ! [ -x "$(command -v platformio)" ]; then
  echo 'Error: platformio is not installed.' >&2
  exit 1
fi

function deploy_firmware () {
    # TODO
}

function deploy_to_satellite () {
    full_satname = "{$1} satellite"
    echo "Deploying code now to $full_satname. Ensure that $full_satname is UNPLUGGED from computer."

### Preflight reset ####

    echo  "First we'll reset the $full_satname's computers to a safe initial state."
    input "1. Plug in the $full_satname's flight computer now. Press ENTER when plugged in."
    echo  "   Restoring $full_satname's flight computer to preflight configuration."
    deploy_firmware "fsw_preflight_$1"
    input "2. Unplug the $full_satname's flight computer. Press ENTER when complete."
    
    input "3. Plug in the $full_satname's ADCS computer now. Press ENTER when complete."
    echo "   Restoring $full_satname's ADCS computer to preflight configuration."
    deploy_firmware "adcs_preflight_$1"
    input "4. Unplug the $full_satname's ADCS computer. Press ENTER when complete."

    input "5. Power on the $full_satname to allow the preflight reset code to run. A light will turn on to signal completion. Press ENTER when complete."
    input "6. Power off the satellite. Press ENTER when complete."

### Flight software programming ###
    echo  "-----------------"

    echo  "Now we'll load flight software on to the $full_satname's computers."
    input "7. Plug in the $full_satname's ADCS computer now. Press ENTER when plugged in."
    echo  "   Loading flight software to $full_satname's ADCS computer."
    deploy_firmware "adcs_flight_$1"
    input "8. Unplug the $full_satname's ADCS computer. Press ENTER when complete."

    input "9. Plug in the $full_satname's flight computer now. Press ENTER when plugged in."
    echo  "   Loading flight software to $full_satname's flight computer."
    deploy_firmware "fsw_flight_$1"
    input "10. Unplug the $full_satname's flight computer. Press ENTER when complete."

### Complete! ###

    echo "The $full_satname is now programmed. DO NOT turn it on prior to flight."
    echo "--------------------------------------"
}

function deploy_gsw() {
    echo "Shutting off mission control software."
    echo "Deploying downlink parser asset to mission control."
    echo "Deplying uplink parser asset to mission control."
    echo "Starting mission control software."
}

echo "Generating release files. This might take up to 10 minutes."
releasedir= $(dirname "$0")
echo $releasedir
releasepath= $releasedir/generate_release.sh
echo $releasepath
./releasepath
if proc != 0:
    echo "Error generating release. Run tools/generate_release.sh manually to see why."
    echo 1
echo "Completed release generation."

echo "Downloading Teensy deployment tool."
git clone https://github.com/PaulStoffregen/teensy_loader_cli
cd teensy_loader_cli
make
cd ..
cp teensy_loader_cli/teensy_loader_cli loader
rm -rf teensy_loader_cli/

deploy_to_satellite "leader"
deploy_to_satellite "follower"
deploy_gsw

rm loader
