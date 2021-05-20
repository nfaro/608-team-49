SONG ::= TIMES "#" NOTES "#" DURATIONS;
TIMES ::= (INT "\n")+;
NOTES ::= (INT "\n")+;
DURATIONS ::= (INT "\n")+;
INT ::= [0-9]+;