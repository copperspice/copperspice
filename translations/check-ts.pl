#! /usr/bin/perl -w
#############################################################################
##
## Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
## All rights reserved.
## Contact: Nokia Corporation (qt-info@nokia.com)
##
## This file is part of the translations module of the Qt Toolkit.
##
## $QT_BEGIN_LICENSE:LGPL$
## GNU Lesser General Public License Usage
## This file may be used under the terms of the GNU Lesser General Public
## License version 2.1 as published by the Free Software Foundation and
## appearing in the file LICENSE.LGPL included in the packaging of this
## file. Please review the following information to ensure the GNU Lesser
## General Public License version 2.1 requirements will be met:
## http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
##
## In addition, as a special exception, Nokia gives you certain additional
## rights. These rights are described in the Nokia Qt LGPL Exception
## version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU General
## Public License version 3.0 as published by the Free Software Foundation
## and appearing in the file LICENSE.GPL included in the packaging of this
## file. Please review the following information to ensure the GNU General
## Public License version 3.0 requirements will be met:
## http://www.gnu.org/copyleft/gpl.html.
##
## Other Usage
## Alternatively, this file may be used in accordance with the terms and
## conditions contained in a signed written agreement between you and Nokia.
##
##
##
##
##
## $QT_END_LICENSE$
##
#############################################################################


use strict;

my @groups = ("qt", "assistant", "designer", "linguist", "qt_help", "qtconfig", "qvfb");

my %scores = ();
my %langs = ();

my $files = join("\n", <*_??.ts>);
my $res = `xmlpatterns -param files=\"$files\" check-ts.xq`;
for my $i (split(/ /, $res)) {
  $i =~ /^([^.]+)\.ts:(.*)$/;
  my ($fn, $pc) = ($1, $2);
  for my $g (@groups) {
    if ($fn =~ /^${g}_((.._)?..)$/) {
      my $lang = $1;
      $scores{$g}{$lang} = $pc;
      $langs{$lang} = 1;
      last;
    }
  }
}

my $code = "";

print "L10n  ";
for my $g (@groups) {
  print " ".$g." ";
}
print "\n";
for my $lang (sort(keys(%langs))) {
  printf "%-5s ", $lang;
  my $qt = 1;
  my $rest = 1;
  my $line = "";
  for my $g (@groups) {
    my $pc = $scores{$g}{$lang};
    $pc = "0" if !defined($pc);
    if (int($pc) < 98 or !$qt) {
      if ($g eq "qt") {
        $qt = 0;
      } else {
        $rest = 0;
      }
    } else {
      $line .= " ".$g."_".$lang.".ts";
    }
    printf " %-".(length($g)+1)."s", $pc;
  }
  if ($qt) {
    $code .= " \\\n   ".$line;
    if (!$rest) {
      print "   (partial)";
    }
  } else {
    print "   (excluded)";
  }
  print "\n";
}

my $fn = "translations.pro";
my $nfn = $fn."new";
open IN, $fn or die;
open OUT, ">".$nfn or die;
while (1) {
  $_ = <IN>;
  last if (/^TRANSLATIONS /);
  print OUT $_;
}
while ($_ =~ /\\\n$/) {
  $_ = <IN>;
}
print OUT "TRANSLATIONS =".$code."\n";
while (<IN>) {
  print OUT $_;
}
close OUT;
close IN;
rename $nfn, $fn;
