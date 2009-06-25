#!/bin/sh
#
# Init script for Tracker
#

DAEMON=/usr/lib/tracker/trackerd
NAME=trackerd
DESC=Tracker
OPTIONS=""

test -x $DAEMON || exit 0

. /lib/lsb/init-functions

case "$1" in
  start)
        log_begin_msg "Starting $DESC..."
	$DAEMON $OPTIONS
        log_end_msg $?
        ;;

  stop)
        log_begin_msg "Stopping $DESC..."
	kill -SIGINT `ps aux |grep lib/tracker |awk '{print $1}' `
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
