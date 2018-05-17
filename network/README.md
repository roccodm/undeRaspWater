= Networking scripts =

A small script to provied information on networking performance

== Usage ==

You need `iperf3`, `ping`, `sed`, `tr` installed on your machine and the raspberry PI.

- On the RPI, run: `iperf3 -s -J` (`-J` ensures json output)
- On your computer, run: `sh network_test.sh RPI_IP` (where `RPI_IP` is the network address of the raspberry PI)
- Wait for script completion, you will get an output like: `Written log file to: network_test-TIMESTAMP.log`.
  That file will contians JSON data on the network tests.

You can invert the commands between the RPI and your computer if you feel the RPI might suffer when running as server.
It would be, in any case, good to make a comparison between the two logs.
