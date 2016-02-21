#!/usr/bin/perl
use warnings;
use strict;

# Generate an LED gamma-correction table for Arduino sketches.
# https://learn.adafruit.com/led-tricks-gamma-correction/the-longer-fix

my ( $gamma, $max_in, $max_out ) = @ARGV;
 
$gamma   ||= 2.8;	# Correction factor
$max_in  ||= 255,	# Top end of INPUT range
$max_out ||= 255;	# Top end of OUTPUT range

print "// gamma.pl $gamma $max_in $max_out\n"; 
print "const uint8_t PROGMEM gamma[] = {";
foreach my $i ( 0 .. $max_in ) {
	print "," if $i > 0;
	print "\n  " if $i % 15 == 0;
	printf("%3d", int(
		( ($i / $max_in) ** $gamma ) * $max_out + 0.5)
	);
}
print " };\n";

