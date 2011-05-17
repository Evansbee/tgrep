#! /usr/bin/perl -w

use strict;

my $start_time    =  6 * 3600 + 52 * 60; # 6:52am
my $end_time      = 31 * 3600 + 13 * 60; # 7:13am the next day

my $target_size   = 10; #in MB

if($#ARGV == 0)
{
   $target_size = $ARGV[0];
}

my $total_entries = ($target_size * 1024 * 1024) / 50; #conver to mb, divide by bytes/line
my $avg_step      = (3600 * 24)/$total_entries;
my $current_line  = 1;
my $t = $start_time;
while($t <= $end_time) {
    if ($t < 86400) {
        print "Feb  9 ";
    } else {
        print "Feb 10 ";
    }
    my $h = $t % 86400 / 3600;
    my $m = $t %  3600 /   60;
    my $s = $t %    60;
    printf "%0.2d:%0.2d:%0.2d  | ", $h, $m, $s;
    printf "%d.%d.%d.%d | ", rand() * 255, rand() * 255, rand() *255, rand() * 255;
    printf "This is line number %0.14d", $current_line;
    
    print "\n";
    
    $current_line += 1;
    $t += $avg_step * 0.9;
    $t += rand($avg_step * 0.2);
}



