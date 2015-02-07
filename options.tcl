frame .o

# label .o.scaleLbl -text "Time Span (Days)"
# scale .o.scale -orient horizontal -length 350 -from 1.0 -to 100.0 -variable timespan

label .o.offsetLbl -text "Days Ago"
scale .o.offset -orient horizontal -length 350 -from 100.0 -to 0.0 -variable offset -command { getoffset }

label .o.currentRateLbl -text "Hourly Avg (GPH)"
label .o.currentRate -textvar hourlyAverage

# grid .o.scaleLbl -column 0 -row 0
# grid .o.scale -column 1 -row 0
grid .o.offsetLbl -column 0 -row 1
grid .o.offset -column 1 -row 1
grid .o.currentRateLbl -column 0 -row 2
grid .o.currentRate -column 1 -row 2

pack .o -expand yes

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

proc getoffset { { offset 0 } } {
    set now [ clock seconds ]
    set back [ expr $offset * 86400 ]
    set date [ clock format [ expr $now - $back ] -format data/%Y-%m-%d.dat ]
    plot $date
}

.o.offset configure -from [ eval maxoffset ]
