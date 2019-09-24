# dual-dhclient
Drop-in dhclient replacement daemon which spawns separate IPv4 and IPv6 dhclients

usage: dual-dhclient ifname

This spawns
/sbin/dhclient -p /var/run/dhclient/dhclient.$ifname.4.pid $ifname
and
/usr/local/sbin/dhclient -6 -nw -cf /dev/null -pf /var/run/dhclient/dhclient.$ifname.6.pid $ifname
and writes its own pidfile (after daemonizing) to
/var/run/dhclient/dhclient.$ifname.pid

When it receives a SIGTERM, dual-dhclient uses the .4.pid and .6.pid files
to signal the two dhclient binaries to terminate.

We assume that /sbin/dhclient is FreeBSD's dhclient, while the binary at
/usr/local/sbin/dhclient is the ISC dhclient.

This was written to allow FreeBSD/EC2 instances to use DHCP to get both
IPv4 and IPv6 addresses.
