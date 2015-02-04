array set params {
    width    400
    height   200
    yscale    70
}

set yvals [ list 10 20 30 40 50 60 70 50 20 10 70 1 ]
set delt [ expr double($params(width)) / ( [ llength $yvals ] - 1 ) ]

set stretchy [ expr $params(height) / double($params(yscale)) ]

canvas .c -width $params(width) -height $params(height) -xscrollincrement 1 -bg black
bind . <Destroy> { exit }


set prevy 0
set prevt 0
set t 0

foreach { yval } $yvals {
    set yval [ expr (($params(yscale) - $yval * $stretchy) + $params(height) - 70) ]
    .c create line $prevt $prevy $t $yval -fill white
    set prevt $t
    set prevy $yval
    set t [expr $t + $delt]
}

pack .c
