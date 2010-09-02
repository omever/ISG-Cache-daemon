#!/usr/bin/perl -w

use strict;
use IO::Socket::UNIX;

my $num_proc = 1;
my $num_query = 1;

# Preforking

for(my $i=0; $i<$num_proc; $i++)
{
	last unless(fork());
}

# Now reading data

my $k = $num_query;

srand (time ^ $$);
while($k--) {
	my $socket = new IO::Socket::UNIX(Type => SOCK_STREAM, Peer => '/tmp/l.sock');
	die "Error: $@" unless $socket;
	my $r = int(rand(3000));
	$socket->send(qq{<?xml version="1.0"?>\n});
#$socket->send(qq{<query><raw_sql sql="select * from bill_user where login=:1 and user_type=:2"><param name='arg' value='om'/><param name='arg' value='homenet'/></raw_sql></query>});
	$socket->send(
qq{<query>
	<get_status_description status='on'/>
</query>});
	$socket->shutdown(1);

	my $data;
	while($socket->recv($data, 1024))
	{
		print $data, "\n";
	}

	undef($socket);
}
