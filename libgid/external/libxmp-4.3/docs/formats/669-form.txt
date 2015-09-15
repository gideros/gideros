669 and Extended 669 file format:

offset         |length&type    |description
---------------+---------------+----------------------------------------------
0              |2 bytes        |marker, 'if' for 669, 'JN' for Extended 669
2              |108 bytes      |song message
0x6e           |1 byte         |NOS = number of samples saved (0-64)
0x6f           |1 byte         |NOP = number of patterns saved (0-128)
0x70           |1 byte         |loop order number
0x71           |0x80 bytes     |order list
0xf1           |0x80 bytes     |tempo list for patterns
0x171          |0x80 bytes     |break location list for patterns
0x1f1          |NOS*size struct|sample data
               | samp          |
+--------------+---------------+
|struct samp {
|  13 bytes ASCIIZ filename of instrument
|  1 dword length of instrument
|  1 dword offset of beginning of loop
|  1 dword offset of end of loop
|} smp[NOS]
+--------------+---------------+----------------------------------------------
0x1f1+         |NOP*0x600      |patterns
(NOS*0x19)     |               |
+--------------+---------------+
|this is repeated 8 times for every row and the array of 8 of these is
| repeated 64 times for each pattern.
|
| bits:
|   BYTE[0]:             BYTE[1]:            BYTE[2]:
|  aaaaaaaa             bbbbbbbb            cccccccc
|  +----++-----------------++--+            +--++--+
|  |     |                  |               |   |
|  |     |                  4 bit volume    |   command value
|  |     |                                  |
|  |     aabbbb = 6 bit instrument number   command:
|  |                                         0 = a
|  note value = (12*oct)+note                1 = b
|                                            2 = c
|  special values for byte 0:                3 = d
|    0xfe = no note, only volume change      4 = e
|    0xff = no note or volume change         5 = f
|
|                                           special value for byte 2:
|                                             0xff = no command
|
+--------------+---------------+----------------------------------------------
0x1f1+         |rest of file   |samples
(NOS*0x19)+    |               |
(NOP*0x600)    |               |


  There are six special commands you can enter. To enter a special command hit
  Backspace in the pattern edit screen, enter a letter (a-f) then a number
  (1-F). The first 5 commands alter the way the note will be played, the
  sixth, 'f', changes the tempo in the pattern and has no effect on the note
  being played. All commands except 'c' can be specified as part of a note or
  alone. If they are specified alone, their effect starts where they were
  placed, not when the note was struck. If no instrument is playing on the
  channel where the command was encountered, there will be no effect (except
  for command 'f', it always changes the tempo). The commands continue to
  affect the way the instrument is played untill another note or command
  is encountered in the pattern (an 'f' command will cancel the effects of
  any previous command, but nothing cancels the effect of the 'f' command).
  The format of the commands is c#, where c is the command and # is the
  command value which is the user defined parameter for how much the command
  will affect the instrument.


669 Commands:
=============

    a - Portamento up - This command will cause the frequency of the note to
        increase over time, the command value indicates how fast the pitch
        will increase. (For those of you who know what this means, the port
        is linear, not logarithmic).

    b - Portamento down - Same as 'a' but in the other direction.

    c - Port to note - This is the only command that requires there to be a
        note on the same line. This commands sets the note to portamento at
        the speed defined by the command value towards the note on the line.
        The instrument value of the note is ignored, but the volume is set
        to the volume in the note on this line. When the note reaches the
        destination note, the portamento is stopped and it continues playing
        at the destination note frequency.

    d - Frequency adjust - This command adjusts the frequency of the note
        currently playing a little bit. This is useful for when you have 2
        notes playing at the same pitch using the same instrument, use this
        command to adjust the frequency of one of the notes to make it sound
        a little more harmonic.

    e - Frequency vibrato - This command sets the frequency of the note that
        is playing to vibrate. The command value specifies how much to
        vibrato the note by.

Note: A command value of 0 on any of the previous commands cancels the effect
  of any previous command and sets the note to play normally.

    f - Set tempo - Usually you will set the tempo for the patterns through
        their basic tempo. But there are times when you may want to change
        the tempo within a pattern. Use this command to do it. The tempo will
        remain at this tempo untill another set tempo command or untill
        another pattern is reached in the order list (even the same pattern
        that is playing now).


Extended 669 Commands:
======================

    f0 - Super Fast Tempo. In COMPOSD1, this tempo is unused,
         in UNIS 669 Extended mode, this is a super fast tempo.

    g  - G of Commands
         Subcommand:
         0 - Balance fine slide Left        8 - Unused
         1 - Balance fine slide Right       9 - Unused
         2 - Unused                        10 - Unused
         3 - Unused                        11 - Unused
         4 - Unused                        12 - Unused
         5 - Unused                        13 - Unused
         6 - Unused                        14 - Unused
         7 - Unused                        15 - Unused

    h  - Slot Retrig. This command rapidly fires 4 slots.
         The command parameter specifies the speed at which to do it.
         The speed difference across the values is exponential.

Info from: - 669.DOC and COMPOSD.DOC by Tran
           - UNIS669.HLP by Jason Nunn


Prime/Inertia <sdanes@marvels.hacktic.nl>
