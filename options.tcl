bind . <Destroy> { exit }

frame .o

canvas .o.c -width $params(width) -height $params(height) -xscrollincrement 1 -bg black

label .o.offsetLbl -text "Days Ago"
scale .o.offset -orient horizontal -length 200 -from 100.0 -to 0.0 -variable offset

label .o.currentRateLbl -text "Hourly Avg (GPH)"
label .o.currentRate -textvar hourlyAverage

grid .o.c -column 0 -row 0 -columnspan 2 -sticky nsew
grid .o.offsetLbl -column 0 -row 1
grid .o.offset -column 1 -row 1
grid .o.currentRateLbl -column 0 -row 2
grid .o.currentRate -column 1 -row 2

grid columnconfigure .o 0 -weight 1
grid rowconfigure .o 0 -weight 1
grid columnconfigure .o 1 -weight 1

pack .o -expand yes -fill both

set timespan 1.0
set hourlyAverage 0.450

proc maxoffset {} {
    set now [ clock seconds ]
    for { set i 0 } { $i < 100 } { incr i } {
        set back [ expr $i * 86400 ]
        set date [ clock format [ expr $now - $back ] -format data/%Y-%m-%d.dat ]
        if ![ file exists $date ] {
            return [ expr $i - 1 ]
        }
    }
}

proc render { { offset 0 } } {
    set now [ clock seconds ]
    set back [ expr $offset * 86400 ]
    set date [ clock format [ expr $now - $back ] -format data/%Y-%m-%d.dat ]
    plot $date
}

.o.offset configure -from [ eval maxoffset ]

bind .o.c <Configure> {
    array set params [ list width %w height %h ]
}

bind .o.offset <ButtonRelease-1> { render $offset }
