# Furnace Monitor Application in Tcl/Tk

PC status panel for monitoring a boiler system, works in conjunction with a simple raw serial terminal protocol sending on and off states. Provides basic logging of time run in various intervals, as well as a realtime generated plot of a given interval.

You can use a relay which takes the HV and closes a low voltage circuit depending on whether current is being drawn for the burner. Essentially any device which can determine if the burner is running, without disturbing the normal operation, will work.

# Notice

Working on a boiler system can be dangerous. Make sure that the overpressure valve, aquastat, photocell, etc. are working properly and do not attempt to circumvent any of the safety devices.
