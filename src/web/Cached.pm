package Cached;

require Config::IniFiles;
require Socket;
require IO::Handle;
require XML::DOM;
require XML::Parser::PerlSAX;
use IO::Socket::UNIX;
use Data::Dumper;

sub new 
{
    my $class = shift;
    my $charset = shift;
    my $socket = shift;
    
    my $data = { 'charset' => 'utf8' };
    
    $data->{socketPath} = $socket if $socket;
    $data->{charset} = $charset if $charset; 
    
    unless($data->{socketPath}) {
	my $cfg = new Config::IniFiles( -file => '/etc/cached.ini' );
	$data->{socketPath} = $cfg->val('MAIN', 'socket');
    }
    
    bless $data, $class;
}

sub connect 
{
    my $self = shift;
    $self->{socket} = new IO::Socket::UNIX(Peer => $self->{socketPath}, Type=>IO::Socket::UNIX::SOCK_STREAM) || die $!;
}

sub disconnect 
{
    my $self = shift;
    
    close($self->{socket});
}

sub query 
{
    my $self = shift;
    my $data = shift;
    $self->connect();
    
    $self->{socket}->send($data);
    $self->{socket}->shutdown(1);
    

    my $reply = '';
    %{$self->{rv}} = ();
    $self->{level} = 0;
    $self->{rc} = 0;
    @{$self->{rcstack}} = ();
    $self->{tagname} = '';
    $self->{tc} = 0;

    $self->{parser} = new XML::Parser::PerlSAX(Handler => $self);
    $self->{parser}->parse(Source => {ByteStream => $self->{socket}});    
    undef($self->{parser});
    $self->disconnect();
    
    return $reply;
}

sub start_element
{
    my ($self, $element) = @_;

    $self->{level}++;
    
    my %attrs = %{$element->{Attributes}};
    $current_element = $element->{Name};
    
    if($self->{level} == 2 && lc($current_element) eq 'bind') {
	push @{$self->{rcstack}}, $self->{rc};
	$self->{rc} = 'bind';
    }
    if($self->{level} == 3) {
	if(!defined $self->{tagname} || $self->{tagname} ne $current_element) {
	    $self->{tc} = 0;
	    $self->{tagname} = $current_element;
	} else {
	    $self->{rv}{$self->{rc}}{$self->{tagname}}[$self->{tc}] = '';
	}
    }
}

sub end_element
{
    my ($self, $element) = @_;
    $self->{level}--;
    $current_element = $element->{Name};

    if($self->{level} == 1 && $current_element eq 'branch') {
	$self->{rc}++;
    } elsif($self->{level} == 1 && lc($current_element) eq 'bind') {
	$self->{rc} = pop(@{$self->{rcstack}});
    } elsif($self->{level} == 2) {
	$self->{tc}++;
    }
}

sub characters
{
    my ($self, $chars) = @_;
    return unless $self->{level} == 3;
    my $text = $chars->{Data};
#    print $characters;
    $text =~ s/^\s*//;
    $text =~ s/\s*$//;

    return unless $self->{tagname};
    $self->{rv}{$self->{rc}}{$self->{tagname}}[$self->{tc}] .= $text;
}

sub sql
{
    my $self = shift;
    my $sql = shift;
    my $args = shift;
    
    my $doc = XML::DOM::Document->new;
    my $decl = $doc->createXMLDecl("1.0");
    
    my $query = $doc->createElement('query');
    
    my $raw_sql = $doc->createElement('raw_sql');
    $query->appendChild($raw_sql);
    
    $raw_sql->setAttribute('sql', $sql);

    foreach my $key (keys %$args) {
	my $attr = $doc->createElement('param');
	$attr->setAttribute('name', $key);
	$attr->setAttribute('value', $args->{$key});
	$raw_sql->appendChild($attr);
    }
    
    $self->query($decl->toString() ."\n". $query->toString());
    
    return $self->{rv};
}

1;