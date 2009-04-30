#!/bin/sh
#
# Init script for Tracker
#

DAEMON=/usr/lib/tracker/trackerd
NAME=trackerd
DESC=Tracker
OPTIONS="-v 1"
PIDFILE=/tmp/af-piddir/trackerd.pid

# How many times do we try to restart a dead trackerd.
RESTART_TRIES=2

test -x $DAEMON || exit 0

. /lib/lsb/init-functions

case "$1" in
  start)
        log_begin_msg "Starting $DESC..."
	if [ ! -e /targets/links/scratchbox.config ]; then
	        /usr/sbin/dsmetool -f "$DAEMON $OPTIONS" -c $RESTART_TRIES
	else
		/sbin/start-stop-daemon --start --quiet -p $PIDFILE -m -b -x $DAEMON -- $OPTION
	fi
        log_end_msg $?
        ;;

  stop)
        log_begin_msg "Stopping $DESC..."
	if [ ! -e /targets/links/scratchbox.config ]; then
		/usr/sbin/dsmetool -k "$DAEMON $OPTIONS"
	else
		/sbin/start-stop-daemon --stop --quiet -p $PIDFILE 
	fi
        log_end_msg $?
        ;;
  restart|force-reload)
        $0 stop
        sleep 1
        $0 start
        ;;
  *)
        N=/etc/osso-af-init/$NAME
        log_success_msg "Usage: $N {start|stop|restart|force-reload}" >&2
        exit 1
        ;;
esac

#exit 0  
