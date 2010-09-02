#!/usr/bin/perl -w

use strict;
use IO::Socket::UNIX;

my $num_proc = 3;
my $num_query = 10;

# Preforking

for(my $i=0; $i<$num_proc; $i++)
{
	last unless(fork());
}

# Now reading data

my $k = $num_query;

srand (time ^ $$);

while($k--) {
	my $socket = new IO::Socket::UNIX(Type => SOCK_STREAM, Peer => '/tmp/cached.sock');
	die "Error: $@" unless $socket;
	my $r = int(rand(3000));
	$socket->send(qq{<?xml version="1.0"?>\n});
$socket->send(qq{
<query>
    <raw_sql sql="begin test_om(:out); end;">
    </raw_sql>
</query>});
=here
	$socket->send(
qq{<query>
	<sql sql="begin
			pkg_plan.plan_change(:1, :2);
			end;">
			<param name='arg' value='780486'/>
			<param name='arg' value='2300'/>
	</raw_sql>
</query>});
=cut
	$socket->shutdown(1);

	my $data;
	while($socket->recv($data, 1024))
	{
		print $data, "\n";
	}

	undef($socket);
}
