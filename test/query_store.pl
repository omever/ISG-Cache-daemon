#!/usr/bin/perl -w

use strict;
use IO::Socket::UNIX;

my $num_proc = 60;
my $num_query = 1000;

# Preforking

for(my $i=0; $i<$num_proc; $i++)
{
	last unless(fork());
}

# Now reading data

my $k = $num_query;

srand (time ^ $$);
while($k--) {
	my $socket = new IO::Socket::UNIX(Type => SOCK_STREAM, Peer => '/tmp/test.sock');
	die "Error: $@" unless $socket;
	my $r = int(rand(30000000));
	$socket->send(qq{<?xml version="1.0"?>
	<query><store key="blablabla$r" value="asdfasdfasfdasfdasdfasdf;lakjg;vqefhbpadsofivha;vlkhe;fvlkjdf;lakjdsfgb;kajsdhvlk;asdjvha;slka;dshasldkfjashdlfakjfg" timeout='10'/>
	</query>\n});
	$socket->shutdown(1);

	my $data;
	while($socket->recv($data, 1024))
	{
#		print $data, "\n";
	}

	undef($socket);
}
