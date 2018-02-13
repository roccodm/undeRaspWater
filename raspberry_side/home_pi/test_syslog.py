import syslog

syslog.syslog('Processing started')
syslog.syslog(syslog.LOG_ERR, "Underaspdaemon: voltage under limit")

