labelframe .timespan -text "Time Span"
radiobutton .timespan.daily -text Daily -var timespan -val d
radiobutton .timespan.weekly -text Weekly -var timespan -val w
radiobutton .timespan.biweekly -text Biweekly -var timespan -val bw
radiobutton .timespan.monthly -text Monthly -var timespan -val m
radiobutton .timespan.bimonthly -text Bimonthly -var timespan -val bm

pack .timespan.daily .timespan -side left
pack .timespan.weekly .timespan -side left
pack .timespan.biweekly .timespan -side left
pack .timespan.monthly .timespan -side left
pack .timespan.bimonthly .timespan -side left
pack .timespan -expand yes

set timespan d
