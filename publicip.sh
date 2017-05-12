#!/bin/bash
printf "Public IP: " 
dig +short myip.opendns.com @resolver1.opendns.com
printf "\n"
