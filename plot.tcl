array set params {
    width    500
    height   200
    yscale    60
    step      10
}

bind . <Destroy> { exit }

proc plot { datafile } {
    if ![ file exists $datafile ] {
        return
    }

    global params

    .o.c delete all

    set fp [open $datafile r]
    set yvals [read $fp]
    close $fp

    set delt [ expr ($params(width) / 23) + 0.75]

    set stretchy [ expr $params(height) / double($params(yscale)) ]

    set prevy 0
    set prevt 0
    set t 0
    set started_plotting false

    for { set i 0 } { $i < $params(yscale) } { incr i $params(step) } {
        .o.c create line 0 [ expr $i * $stretchy ] $params(width) [ expr $i * $stretchy ] -fill gray
    }

    foreach { ytime yval } $yvals {
        if [ expr $yval == 0 && $started_plotting == false ] { continue }
        set t [expr $ytime * $delt]
        set yval [ expr (($params(yscale) - $yval * $stretchy) + $params(height) - $params(yscale)) ]
        if [ expr $started_plotting == false ] {
            set prevy $yval
            set prevt $t
            set started_plotting true
        }
        .o.c create line $prevt $prevy $t $yval -fill white
        set prevt $t
        set prevy $yval
    }

}
