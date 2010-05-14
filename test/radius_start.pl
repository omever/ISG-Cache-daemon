#!/usr/bin/perl -w

use strict;
use IO::Socket::UNIX;

# Now reading data

my $socket = new IO::Socket::UNIX(Type => SOCK_STREAM, Peer => '/tmp/test.sock');
die "Error: $@" unless $socket;
my $r = int(rand(3000));
$socket->send(qq{<?xml version="1.0"?>\n});
$socket->send(qq{<query>
	<radacct>
	<Acct-Session-Id>000215E4</Acct-Session-Id>
	<Framed-IP-Address>217.113.121.85</Framed-IP-Address>
	<Framed-Protocol>PPP</Framed-Protocol>
	<User-Name>217.113.121.85</User-Name>
	<Cisco-AVPair>connect-progress=Call Up</Cisco-AVPair>
	<Cisco-Control-Info>I0;57925</Cisco-Control-Info>
	<Cisco-Control-Info>O0;198199</Cisco-Control-Info>
	<Cisco-AVPair>nas-tx-speed=77266392</Cisco-AVPair>
	<Cisco-AVPair>nas-rx-speed=6645676</Cisco-AVPair>
	<Acct-Session-Time>39876</Acct-Session-Time>
	<Acct-Input-Octets>57925</Acct-Input-Octets>
	<Acct-Output-Octets>198199</Acct-Output-Octets>
	<Acct-Input-Packets>441</Acct-Input-Packets>
	<Acct-Output-Packets>484</Acct-Output-Packets>
	<Acct-Authentic>RADIUS</Acct-Authentic>
	<Acct-Status-Type>Interim-Update</Acct-Status-Type>
	<NAS-Port-Type>33</NAS-Port-Type>
	<Cisco-NAS-Port>0/0/3/71</Cisco-NAS-Port>
	<NAS-Port-Id>0/0/3/71</NAS-Port-Id>
	<Service-Type>Framed-User</Service-Type>
	<NAS-IP-Address>217.113.112.11</NAS-IP-Address>
	<Event-Timestamp>Фев 15 2009 16:46:28 SAMT</Event-Timestamp>
	<NAS-Identifier>Spike:217.113.112.011</NAS-Identifier>
	<Acct-Delay-Time>0</Acct-Delay-Time>
	<Acct-Unique-Session-Id>48c6af4046d25476</Acct-Unique-Session-Id>
	<Timestamp>1234728000</Timestamp>
	<Request-Authenticator>Verified</Request-Authenticator>
	<Cisco-Service-Info>NINTERNET</Cisco-Service-Info>
	</radacct>
</query>});
$socket->shutdown(1);

my $data;
while($socket->recv($data, 1024))
{
	print $data, "\n";
}

undef($socket);
