array set params {
    width    500
    height   200
    yscale    70
    step      10
}

set fp [open "list.dat" r]
set yvals [read $fp]
close $fp

set delt [ expr double($params(width)) / ( [ llength $yvals ] - 1 ) ]

set stretchy [ expr $params(height) / double($params(yscale)) ]

canvas .c -width $params(width) -height $params(height) -xscrollincrement 1 -bg black
bind . <Destroy> { exit }

set prevy 0
set prevt 0
set t 0

for { set i 0 } { $i < 70 } { incr i $params(step) } {
    .c create line 0 [ expr $i * $stretchy ] $params(width) [ expr $i * $stretchy ] -fill gray
}

foreach { yval } $yvals {
    set yval [ expr (($params(yscale) - $yval * $stretchy) + $params(height) - 70) ]
    .c create line $prevt $prevy $t $yval -fill white
    set prevt $t
    set prevy $yval
    set t [expr $t + $delt]
}

pack .c
