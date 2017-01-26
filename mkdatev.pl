#!/usr/bin/perl

$now = time;
($sc,$mn,$nhr,$ndy,$nmo,$nyr,$nwday,$nyday,$nisdst) = localtime($now);
$nyr = $nyr+1900; $nmo = $nmo+1;

print "`define DATESTAMP 32\'h";
printf("%04d%02d%02d\n", $nyr, $nmo, $ndy);

