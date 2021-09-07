#!/bin/bash

#This script boots ElasticSearch and brings up the status window for reference

sudo systemctl start elasticsearch.service
sudo systemctl status elasticsearch.service