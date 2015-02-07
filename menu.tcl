option add *tearOff 0

menu .menu

. configure -menu .menu

menu .file
# .file add command -label "Device" -command "setDevice"
.file add command -label "Exit" -command "exit"

menu .help
.help add command -label "About" -command "helpAbout"

.menu add cascade -menu .file -label File
.menu add cascade -menu .help -label Help
