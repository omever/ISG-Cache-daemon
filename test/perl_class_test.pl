#!/usr/bin/perl -w

use lib '../src/web';
use lib './';
use strict;
use Cached;
use Data::Dumper;

my $cached = new Cached;
# print $cached->query("<?xml version=\"1.0\"?><query><raw_sql sql=\"select 1 from dual\"/></query>");
print Dumper($cached->sql("select * from bill_user where login=:login", { 'login' => 'om' }));
print Dumper($cached->sql("begin test_om(:out); end;"));
