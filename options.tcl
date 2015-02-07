frame .o

label .o.scaleLbl -text "Time Span (Days)"
scale .o.scale -orient horizontal -length 350 -from 1.0 -to 100.0 -variable timespan

label .o.offsetLbl -text "Offset (Days)"
scale .o.offset -orient horizontal -length 350 -from 1.0 -to 100.0 -variable offset

label .o.currentRateLbl -text "Hourly Avg (GPH)"
label .o.currentRate -textvar hourlyAverage

grid .o.scaleLbl -column 0 -row 0
grid .o.scale -column 1 -row 0
grid .o.offsetLbl -column 0 -row 1
grid .o.offset -column 1 -row 1
grid .o.currentRateLbl -column 0 -row 2
grid .o.currentRate -column 1 -row 2

pack .o -expand yes

set timespan 1.0
set hourlyAverage 0.450
