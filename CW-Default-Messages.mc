#
# CW Function Key File
#
# This file can be used in most CW contests that have a simple exchange
# The {EXCH} macro uses the contents of the Sent Exchange box in the contest setup
# The {SENTRSTCUT} defaults to 5NN. If you want to send another signal report,
#  change the Sent RST in the Entry Window before transmitting your exchange.
#
# F5 uses "!" macro for his callsign
# Do not use the {CALL} macro in place of the ! macro
#
# S&P F1 calls QRL? before placing the program in RUN mode for calling CQ
# To respond to caller, send F5 then F2, or ; (default exchange key) or 
#   Insert, or Enter in ESM
#
###################
#   RUN Messages 
###################
F1 Cq,Cq Test {MYCALL} {MYCALL}
F2 Exch,{SENTRSTCUT} {EXCH}
F3 Tu,Tu {MYCALL}
F4 {MYCALL},{MYCALL}
F5 His Call,!
F6 Repeat,{SENTRSTCUT} {EXCH} {EXCH}
F7 Spare,
F8 Agn?,Agn?
F9 Nr?,Nr?
F10 Call?,Cl?
F11 Spare,
F12 Wipe,{WIPE}
#
###################
#   S&P Messages 
###################
F1 Qrl?,Qrl? de {MYCALL}
F2 Exch,{SENTRSTCUT} {EXCH}
F3 Tu,Tu
F4 {MYCALL},{MYCALL}
F5 His Call,!
F6 Repeat,{SENTRSTCUT} {EXCH} {EXCH}
F7 Spare,
F8 Agn?,Agn?
F9 Nr?,Nr?
F10 Call?,Cl?
F11 Spare,
F12 Wipe,{WIPE}
