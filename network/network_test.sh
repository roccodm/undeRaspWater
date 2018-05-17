#/bin/sh

LOG="network_test-`date +%s`.log"

if [ -z "$1" ]; then
	echo "Please specify host: $0 server_host"
	exit 1
fi

HOST="$1"

run() {
	iperf3 -f b -J $1 -t $2 -b $3 -c $HOST --get-server-output
	sleep 3
}

ping_test() {
	ping -c 10 $HOST
}

ping_flood() {
	ping -f -c 10000 $HOST | tr '\b' '<'
}

nonl() {
	sed -e ':a;N;$!ba;s/\n/\\n/g'
}

echo "Log will be written to: $LOG"
echo 'TCP Short 100mb'
echo '{"tcp_short_100": ' > $LOG
run '' 10 100m >> $LOG
echo 'TCP Short 300mb'
echo ', "tcp_short_300": ' >> $LOG
run '' 10 300m >> $LOG

echo 'TCP Long 100mb'
echo ', "tcp_long_100": ' >> $LOG
run '' 30 100m >> $LOG
echo 'TCP Long 300mb'
echo ', "tcp_long_300": ' >> $LOG
run '' 30 300m >> $LOG

echo 'UDP Short 100mb'
echo ', "udp_short_100": ' >> $LOG
run '-u' 10 100m >> $LOG
echo 'UDP Short 300mb'
echo ', "udp_short_300": ' >> $LOG
run '-u' 10 300m >> $LOG

echo 'UDP Long 100mb'
echo ', "udp_long_100": ' >> $LOG
run '-u' 30 100m >> $LOG
echo 'UDP Long 300mb'
echo ', "udp_long_300": ' >> $LOG
run '-u' 30 300m >> $LOG

echo "Ping test"
echo -n ', "ping": ' >> $LOG
PING=`ping_test`
echo "\"$PING\"," | nonl >> $LOG

echo "Ping flood test"
echo -n '"ping_flood": ' >> $LOG
FLOOD=`ping_flood`
echo "\"$FLOOD\"" | nonl >> $LOG
echo -n '}' >> $LOG
echo "Written log file to: $LOG"
chmod a+rw $LOG
