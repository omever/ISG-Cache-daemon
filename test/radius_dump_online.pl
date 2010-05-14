#!/usr/bin/perl -w

use strict;
use IO::Socket::UNIX;

# Now reading data

my $socket = new IO::Socket::UNIX(Type => SOCK_STREAM, Peer => '/tmp/test.sock');
die "Error: $@" unless $socket;
my $r = int(rand(3000));
$socket->send(qq{<?xml version="1.0"?>\n});
$socket->send(qq{<query>
	<cbilling_dump_online/>
</query>});
$socket->shutdown(1);

my $data;
while($socket->recv($data, 1024))
{
	print $data, "\n";
}

undef($socket);
