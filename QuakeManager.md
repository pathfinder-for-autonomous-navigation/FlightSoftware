Quake Manager
------------------------------------------------------------------------------

States: Transceiving, Waiting, Writing, Reading
Assume an implicit call to qct.execute() that occurs before each evaluation of the state machine
Note: (state = IDLE) usually means that we're done with the operation

Fields: 
  - radio.mo_msg_queue: queue of messages we want to transmit
  - radio.mt_msg_queue: queue of messages we received
  - last_checkin_cycle: the cycle at which we last received comms

Error: transition to Waiting, update last_checkin_cycle (hope that it fixes itself in 5 minutes)

Each state is allotted a maximum amount of cycles that they can waste until we transition them back to Waiting.
It is only wasting cycles if execute returns false (i.e. quake didn't accomplish anything this cycle). 
Obviously, reset the cycles_eaten whenever you're about to transition to a new cycle. Also request the corresponding state. 

[Waiting]
  + waiting.cycles_wasted > waiting.cycles_allocated --> transition to Transceiving, state := SBDIX

[Transceiving]
  + sbdix.cycles_wasted > sbdix.cycles_allocated --> transition to Waiting, state := IDLE 

  + state = SBDIX --> // request SBDIX
  + state = IDLE  --> 
                      - No SBDIX response --> sbdix.cycles_wasted++
                      + SBDIX response    --> 
                                               - No comms :(       --> 
                                                                        + sbdix.stutter_tolerance tries have passed      --> transition to Waiting
                                                                        - sbdix.stutter_tolerance tries have not passed  --> sbdix.cycles_wasted++ 
                                               + We have comms :)  -->    update last_checkin_cycle
                                                                        + We have a message!  --> transition to Reading, 
                                                                                                  state := SBDRB
                                                                        - We have no message. --> transition to Writing, 
                                                                                                  state := SBDWB

[Reading]
  + Reading.cycles_wasted > Reading.cycles_allocated --> transition to Waiting

  + state = SBDRB --> sbdrb.cycles_wasted++
  + state = IDLE  --> write message to radio.mt_msg_queue, transition to Writing, state := SBDWB

[Writing]
  + transmit.cycles_wasted > transmit.cycles_allocated --> transition to Waiting

  + state = SBDWB --> sbdwb.cycles_wasted++
  + state = IDLE  --> transition to Transceiving, state := SBDIX

  Compiler Flags
------------------------------------------------------------------------------

Hardware interaction methods and constructors need to be conditionally compiled
with `ifdef DESKTOP` so that for example I2C commands don't actually attepmt to interact
with hardware, and that constructors do not ask for an I2C bus to initialize with.

For PIO unit tests which are run with the `pio test -e <env_name>` command,
the UNIT_TEST flag is automatically set.

Mocking methods/behavior that allow a user to set the return of a hardware call, 
should be conditionally compiled with `ifdef UNIT_TEST`. The consideration of the
`DESKTOP` flag is not necessary because any unit-testing or testing of actual hardware
interaction methods/behavior will be tested with psim testing.
                                             
Take note to make sure that the actual unit test scripts are also conditionally compiled
so that they work in hootl and hitl testing.
