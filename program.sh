#!/bin/sh
set +e
killall curl
set -e
MYHOSTNAME=$(hostname -I |  awk '{print $1}'
)

PIHOST=$(cat $3/testfixturePi)
scp ${2} pi@${PIHOST}:/tmp  # download hex file to pi and supply path in REST call 
curl --header "Content-Type: application/json" --request POST  --data "{\"hexfile\":\"tmp/${2}\", \"mcu\": \"${1}\"}" http://${PIHOST}:3000/program
#curl --header "Content-Type: application/json" --request POST  --data "{\"hexfile\":\"http://pubuntu.hwlab.com:8080/${2}\", \"mcu\": \"${1}\"}" http://pi41t:3000/program

