bind . <Destroy> { exit }

frame .o

canvas .o.c -width $params(width) -height $params(height) -xscrollincrement 1 -bg black

label .o.offsetLbl -text "Days Ago"
scale .o.offset -orient horizontal -length 200 -from 100.0 -to 0.0 -variable offset

label .o.currentRateLbl -text "Average This Hour (Runtime)"
label .o.currentRate -textvar hourlyAverage

label .o.gphLbl -text "Nozzle GPH"
entry .o.gph -textvariable gph -bg white

label .o.currentRateGPHLbl -text "This Hour (Gallons)"
label .o.currentRateGPH -textvar hourlyAverageGPH

grid .o.c -column 0 -row 0 -columnspan 2 -sticky nsew
grid .o.offsetLbl -column 0 -row 1
grid .o.offset -column 1 -row 1
grid .o.currentRateLbl -column 0 -row 2
grid .o.currentRate -column 1 -row 2
grid .o.gphLbl -column 0 -row 3
grid .o.gph -column 1 -row 3
grid .o.currentRateGPHLbl -column 0 -row 4
grid .o.currentRateGPH -column 1 -row 4

grid columnconfigure .o 0 -weight 1
grid rowconfigure .o 0 -weight 1
grid columnconfigure .o 1 -weight 1

pack .o -expand yes -fill both

button .b -text "Refresh" -command { refresh }
pack .b

set timespan 1.0
set hourlyAverage 404
set hourlyAverageGPH 404
set gph 0.85

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

proc refresh {} {
    global offset
    global hourlyAverage
    global hourlyAverageGPH
    global gph
    set now [ clock seconds ]
    set datafile [ clock format [ expr $now ] -format data/%Y-%m-%d.minutes.dat ]
    set current_hour [ clock format [ expr $now ] -format %H ]
    set fp [open $datafile r]
    set seconds [read $fp]
    set total_seconds 0
    set number_mins 0
    close $fp
    foreach { hour minute seconds } $seconds {
        if [ expr $hour == $current_hour ] {
            set total_seconds [ expr $total_seconds + $seconds ]
            incr number_mins
        }
    }
    set hourlyAverage [ expr $total_seconds / $number_mins ]
    set hourlyAverageGPH [ expr $hourlyAverage * $gph ]
    render $offset
}

.o.offset configure -from [ eval maxoffset ]

bind .o.c <Configure> {
    array set params [ list width %w height %h ]
}

bind .o.offset <ButtonRelease-1> { render $offset }
