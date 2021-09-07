#!/bin/bash

#                    Script Desription:
# This Script Boots up the Ground Software Stack in one 
# TMUX instance for easy startup in case of a crash,
# more efficient maintenance, and persistence after logging 
# out of SSH. The Purpose of this is for our Scaleway Server,
# however could be used on any machine in which GSW is installed
#
#
# ***               IMPORTANT Information:              ***
# *** RUN THIS FROM .../FlightSoftware/tools/GSWStartup ***
#
# NOTE: To use this script you must first run 'sudo visudo'
#       Then add 'Default[tab]!tty_tickets' to the top.
#       where [tab] is the tab key
#
#       This allows for sudo auth based on time not 
#       terminal window (necessary for multiple tmux 
#       instances, like we are using here). This will 
#       not only effect tmux but the whole system, so 
#       this script is preferably just for the server.


# Validates sudo for use within tmux instances
sudo --validate

sudo apt install tmux # Installs tmux if it is not installed

#starts ElasticSearch
tmux \
    new-session -s GSW './startes.sh' \; \
    detach-client
clear
echo "Please Wait While ElasticSearch starts up"

# Awaiting ElasticSearch Startup BEFORE starting TLM and MCT
# 30 Seconds is on the very high end for elasticsearch startup time
# However this is important so we don't start tlm before elasticsearch
sleep 30 

tmux attach-session -t GSW \; \
    split-window -h './starttlm.sh' \; \ #starts TLM
    split-window -v './startmct.sh' \; \ #starts MCT
    detach-client
clear

# User Information:

echo "Ground Software is running at tmux session 'GSW'"
echo "To reatach to this instance type 'tmux attach-session -t GSW'"
echo "To detach again do 'Ctrl-B' followed by 'D'. To switch Panes use \
    'Ctrl-B' followed by the arrow key pointing to correct terminal pane"
