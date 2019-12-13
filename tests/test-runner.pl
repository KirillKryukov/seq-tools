#!/usr/bin/env perl
#
# Test runner script
# Copyright (c) 2018-2019 Kirill Kryukov
# See README.md and LICENSE files of this repository
#

use strict;
use File::Basename qw(basename dirname);
use File::Glob qw(:bsd_glob);

$| = 1;

my @tests = @ARGV;
if (!scalar @tests) { die "Tests to run are not specified\n"; }

my $sep = ($^O =~ /MSWin/) ? '\\' : '/';
my $null = ($^O =~ /MSWin/) ? 'nul' : '/dev/null';
my ($n_tests_total, $n_tests_passed) = (0, 0);

my @tools = grep { $_ ne 'common' } map { $_ = basename($_); s/\.c$//; $_; } bsd_glob("..${sep}src${sep}*.c");

foreach my $test (@tests)
{
    if (-e $test and -d $test) { run_test_set($test); }
    else { run_test($test); }
}

if ($n_tests_passed == $n_tests_total)
{
    print "All $n_tests_passed tests passed\n";
}
else
{
    print "$n_tests_passed out of $n_tests_total tests passed\n";
    exit 1;
}

sub run_test_set
{
    my ($dir) = @_;
    print "===== $dir =====\n";
    if (!-e $dir or !-d $dir) { die "Can't find test set directory \"$dir\"\n"; }
    foreach my $file (bsd_glob("$dir${sep}*.test")) { run_test($file); }
}

sub run_test
{
    my ($test_file) = @_;
    my ($dir, $name) = (dirname($test_file), basename($test_file));
    $name =~ s/\.test$//;
    my $test_prefix = $dir . $sep . $name;
    my $group = $name;
    $group =~ s/-.*$//;
    my $group_prefix = $dir . $sep . $group;

    my @ref_files = bsd_glob("$dir${sep}$name.*-ref");
    my $n_ref_files = scalar(@ref_files);

    print "[$dir${sep}$name] ";
    $n_tests_total++;

    my (@cmds, @errors, $TEST_FILE);
    if (!open($TEST_FILE, '<', $test_file)) { push @errors, "Can't open test file \"$test_file\"\n"; }

    if (scalar(@errors) == 0)
    {
        binmode $TEST_FILE;
        while (<$TEST_FILE>)
        {
            s/[\x0D\x0A]+$//;
            if (!/^(\d+)\s+(.+)$/) { push @errors, "Can't parse test file \"$test_file\"\n"; last; }
            my ($expected_error_code, $cmd) = ($1, $2);
            $cmd =~ s/\{TEST\}/$test_prefix/g;
            $cmd =~ s/\{GROUP\}/$group_prefix/g;
            foreach my $tool (@tools) { $cmd =~ s/\{$tool\}/..${sep}bin${sep}$tool/g; }
            push @cmds, $cmd;
            my $error_code = system($cmd);
            if ($error_code != $expected_error_code) { push @errors, "Command returned wrong error code $error_code, expected $expected_error_code:\n$cmd\n"; last; }
        }
        close $TEST_FILE;
    }

    if (scalar(@errors) == 0)
    {
        foreach my $ref_file (@ref_files)
        {
            my $out_file = $ref_file;
            $out_file =~ s/-ref$//;
            if (-e $out_file)
            {
                my $cmperr = system("diff -q $ref_file $out_file >$null");
                if ($cmperr != 0) { push @errors, "\"$out_file\" differs from \"$ref_file\""; }
            }
            else { push @errors, "Can't find output file \"$out_file\""; }
        }
    }

    if (scalar(@errors) == 0)
    {
        print (($n_ref_files == 0) ? "Generated\n" : "OK\n");
        $n_tests_passed++;
    }
    else
    {
        print "FAIL\n";
        print "Commands:\n", join("\n", map { "    $_" } @cmds), "\n";
        print "Errors:\n", join("\n", map { "    $_" } @errors), "\n";
    }
}
