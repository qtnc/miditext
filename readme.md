# Introduction
MidiText is a small program allowing you to create/compose MIDI songs by entering text-based commands.

The commands used in MidiText are like 
[ABC notation](https://www.abcnotation.com/)
or
[MML](https://en.wikipedia.org/wiki/Music_Macro_Language)
though quite different, with the goal to give access to more specific MIDI features.


IF you already have used [ABC](https://www.abcnotation.com/) or MML music notation, the commands are similar in the principle, but only in the principle. The syntax differs quite a lot.

- The main goal of ABC is to stay rather simple, but still look like real musical scores as far as it's possible with ASCII. 
- MML is often quite software-specific or chip-specific, while MidiText is centered on general MIDI

The result is that MidiText syntax much more ressemble programming than music notation or transcription of music scores, and allow much more control / direct access to MIDI specific features.
I don't care at all about displaying music scores.
First of all I'm a developer, and then only sometimes a musician; not the opposite.... sorry!

Still, I hope that you will appreciate this software and produce beautiful music with it. Please contact me if you do !


# Features

- Create your song in the simple, but effective text editor of the program
- Listen to your music almost live as you are composing
- Use BASS MIDI engine to play music, with SF2/SF3/SFZ soundfonts and advanced preset mapping
- Export your creations to OGG or MP3

# Download
Latest download can be found under <http://vrac.quentinc.net/MidiText.zip>

You may want to download MidiText v5, last updated in 2016 under
<http://demo.quentinc.net/Miditext/miditext500b.zip>

# History

I discovered ABC notation and abc2midi program for the first time in 2003.
Although it was cool, it was severly limited: it was difficult to create songs that went beyond a single track of piano or guitar.
Almost no drums, almost only a single instrument or maybe two and that was it.

It was also very cool because traditional music creation software weren't accessible with screen readers. Well known titles like ableton, FL studio, lmms and all others are still almost totally unusable nowadays, and I think they probably won't ever be.
We can, however, use Reaper. That's very nice.
Still, I'm more a developer than a musician, meaning that my head goes much faster than my ffingers. I'm much better at � programming music � than at the traditional way of playing music on real instruments, recording, arranging and mastering. So Reaper isn't completely my cup of tea either.

I would have probably been very interested in creating music with music trackers as known as music modules. However, here too, all software are next to unusable with a screen reader, back in 2000s as well as nowadays including OpenMPT and the like.
Music modules are also quite rigid: a pattern is always exactly 64 rows of notes or commands, and it isn't always very simple to think about your tune mentally. It stays quite a visual way of thinking.
That's why I think that MIDI still stays the best and more flexible format to make music.

Back in 2003 after having discovered ABC and its serious limitations, while, as said, the inability to use other software, I have got the idea to create a language similar to ABC, but allowing more flexibility, more creativity. MidiText was born.

However, except with a couple friends, I have never shared MidiText with the world until now. The friends in question are musician, not programmers, and they all have said that anyway, I'm the only one who can understand my program. Nobody wants to create music by entering text commands, that's just crazy!

Now in 2023, I assume that's still mostly true. But why should I still keep that secret?
There's no reason afterall. I won't ever make money or what with something like that anyway. So I finally decided to publish it. Maybe someone would eventually be interested? we never know.
I'm porting all my old software to WXWidgets, so that's the occasion to finally do it, 20 years after the initial idea. It has well evolved since that time, though I haven't yet ported all the features of MidiText v3, v4 and v5.
MIDI text v5 can be downloaded from <http://demo.quentinc.net/Miditext/miditext500b.zip>

# Example
## Example 1

[demo1.mid](http://vrac.quentinc.net/demo1.mid)
```
T(MidiText demo 1)
M:4/4
K:DM
t120 
L/4
edG2[Bg]r[B2g]edG2[Bg]r[B2g]edG2[B2g]E2[B2g]D2[Af#]r[A2f#]
edD2[Af#]r[A2f#]edD2[Af#]r[A2f#]edD2A&f#2E2[A2f#]G2[Bg]r[B2g] 
```



## Example 2

[demo2.mid](http://vrac.quentinc.net/demo2.mid)
```
title(MidiText Demo 2)
M:4/4
K:CM
t144


V:1
p0 o5
velpattern(100, 85, 90, 85, 75, 1)
g/>g/e/>g/ag
f/>f/d/>f/gf
e/>e/c/>e/fe
d/>d/B/>d/ed
c2 >(3e/4g/4c'3

V:2
p0 o4
v84
velpattern(100, 85, 90, 85, 75, 1)
c [eg]G [eg]
d [fg] G [fg]
c [eg] G [eg]
G [Bd] D [Bd]
[ceg]
G/>A/c

```

## More examples
Another demo:
[demo3.mid](http://vrac.quentinc.net/demo3.mid)
and it's [recorded OGG version](http://vrac.quentinc.net/demo3.ogg)

A full project with source included:
[tpdcv4.7.mid](http://vrac.quentinc.net/tpdcv4.7.mid)
and result with ARachno soundfont
[tpdcv4.7.ogg](http://vrac.quentinc.net/tpdcv4.7.ogg)


# MidiText Syntax
## Notes and length
Notes: optional octave + note letter + optional alteration + optional duration + optional modification

- Notes are indicated by a single letter A-G or a-g. Silences/rests are indicated by letters r, s or z.
- A note can be preceeded by an octave deviation between -10 and 10. This octave number is relative to the current octave of the channel.
- Octaves can also be given in ABC-style, with `'` and `,` characters placed after note and length, e.g. `c,` is equivalent to `C` and `C'` is equivalent to `c`.
- A note can be followed by an alteration: one of `#+^` for sharp, one of `-_` for flat
- A note can be followed by its length/duration, by default when omited the duration is one  quarter note or the default note length if it has been specified
- Duration is indicated as n/d, where n is the numerator and d the denominator. Both can be omited and if so, both are set to 1. 
For example, if we assume that default note length is a quarter note, 2 would indicate an half note (2/1), and /2 whould indicate an eight note (1/2). / alone is a shorten for 1/2.
IF the default note length is  set to an eight note, 4 would represent an half note and /4 a 32th note.
- Notes can be separated with spaces, or glued together as long as there is no ambiguity. Numbers are preferably taken as lengths rather than octave indications.

Examples:

- `C4` is a simple C full note
- `1e/4` is a E one octave above the current octave and is a 16th note, which could also have been written `e/4'`
- `-1B_/` is an eighth note flat B, one octave below the current octave, which could also been written `B_/,`
- `c2d2e2` are three half notes C, D and E played one after the other

## Chors
Chords can be written in two ways:

- Bracketed notation: `[c4eg]`
- Ampersant notation: `c2&e2&g2`

- With the bracketed notation, all notes have the same length given by the first note. 
- With the ampersant notation, all notes may have different lengths and this isn't an error.

## Other notations and rhythms
A few characters have a special meaning when they follow immediately a note length. For that special meaning to take effect, there shouldn't be any space between the note length and the modification characters.

- `>` and `<` denote broken rhythms. `A>B` means `A4/3 B2/3` and `A<B` means `A2/3 B4/3`.
- `!?~` are currently reserved but have no use

## Repeatition and bars

- `|:` marks the beginning of a repeated section
- `|1` marks the first alternation, i.e. the part that will be played the first time 
- `:|2` marks the end of the first alternation and the beginning of the second alternation, i.e. the part that will be played the second time. 2 can be replaced by 3-9 for more alternates repeatitions
- `||` or `|]` or `[|` marks the end of the repeated section
- `(` marks the beginning of a simple repeated section (simple = without alternations)
- `)n` marks the end of a simple repeated section, where n is the number of repeatitions desired (2+)
- `|` Simple bars are ignored, you can use them to improve visual appearance / readability if you like

## Variables
You can use variables where integers are expected, for example: `v$x` sets the note velocity to the value of variable x. Be careful though, you can use variables but not expressions! Variables must be set beforehand.

- To set a variable: $x=127
- Increment a variable: $x=x+1. Note the absence of $ on the right hand side.
- Allowed operators: `+ - * / % ^`
- Allowed functions: math.min/max/floor/log/sin/cos/asin/acos, bit.bor/band/bxor
- Predefined variables: $channel=current channel, $ppq=MIDI ticks per quarter note (default=480)

## Short commands
Short commands: optional octave + single command letter + optional alteration + optional value or duration + modification characters

Short commands include regular notes (with letters A-G a-g), but also several other common command that are frequently used.
The syntax is quite lenient: length, octave, alteration and modifications are always recognized, even with commands where they don't make sense.
Some command lettters have a different signification when a value/length is specified or not.

- A: lower note A
- B: lower note B
- C: lower note C
- D: lower note D
- E: lower note E
- F: lower note F
- G: lower note G
- H: pitch bend sensitivity, in semitones (MIDI RPN 0) (1-127)
- I: unused
- J: portamento start note (0-127) (CC84)
- J: portamento switch off (when no value specified) (CC65)
- K: sustain pedal off (CC64)
- L: default note length / note duration multiplier
- M: unused
- N: unused
- O: unused
- P: absolute time positionning from the beginning of the song
- Q: quiet/soft pedal off (CC67)
- R: negative/backward silence/rest
- S: negative/backward silence/rest
- T: unused
- U: sostenuto pedal off (CC66)
- V: set channel volume (CC7) (0-127)
- W: modulation wheel range (MIDI RPN 5) (0-16383)
- X: unused
- Y: unused
- Z: negative/backward silence/rest
- a: higher note A
- b: higher note B
- c: higher note C
- d: higher note D
- e: higher note E
- f: higher note F
- g: higher note G
- h: set pitch bend value (0-16383)
- i: set instrument / program change (0-2097151). 
- j: portamento time (0-127) (CC5)
- j: portamento switch on (when no value specified) (CC65)
- k: sustain pedal on (CC64)
- l: default note length / note duration multiplier
- m: set maximum note on length
- n: set panning (CC10) (0-127)
- o: absolute octave change (0-10)
- p: set instrument / program change (0-2097151)
- q: quiet/soft pedal on (CC67)
- r: silence/rest
- s: silence/rest
- t: set tempo (BPM) (10-1000)
- u: set channel pressure (0-127)
- u: sostenuto pedal on (when used without parameter) (CC66)
- v: set note velocity (0-127)
- w: set modulation wheel value (CC1) (0-127)
- x: set expression (CC11) (0-127)
- y: unused
- z: silence/rest
- `&`: chord separator
- `<`: octave down, when used as a prefix
- `>`: octave up, when used as a prefix

## Long commands
Long commands: 

- command + ':' + value without spaces
- command + '(' + value + ')'
- command + '{' + value + '}'
- command + '[' + value + ']'

Long commands are again identified by a single letter, but accept longer parameters than just a single value or a length.

- A: key polyphonic aftertouch / key pressure `A:x,y` or `A(x,y)` where x=note, y=value (0-127)
- B: unused
- C or copyright: copyright information (MIDI meta 2)
- D. unused
- E: unused
- F: unused
- G: unused
- H: unused
- I or instrument: instrument name (MIDI meta 4)
- J: unused
- K: key signature, in the form of note+optional alteration + 'M' (major) or 'm' (minor). For example, Gm=G minor, E_M=E flat major
- L or lyric: song lyrics (MIDI meta 5)
- M: time signature (aka meter), in the form numerator/denominator. For example 3/4, 4/4, 6/8
- N: unused
- O: unused
- P or cue: cue point (MIDI meta 7)
- Q: unused
- R or mark: marker (MIDI meta 6)
- S: unused
- T or title: song title (MIDI meta 3)
- U: unused
- V: voice/channel number selection for following commands
- W: unused
- X or text: any text (MIDI meta 1)
- Y: unused
- Z: unused
- i or p: set instrument / program change, in separate bank/program parameters; p(b,n) or p(b,n) where b=bank number (0-16383) and n = instrument/program number (0-127). Also accepts p(x,y,z) where x=bankMSB (0-127), y=BankLSB (0-127), z=program (0-127)

## Elaborate commands
Elaborate commands are like long commands, except that the command is a word rather than just a single letter. The syntax is the same.

- aftertouchdest(t,x).: Aftertouch destination effect (see GM2 doc). t=type (pitch|volume|vibrato|filter), x=value (0-127)
- brokenrhythm(value): Fraction of the broken rhythm. For a construct a>b, a value of /3 means a4/3 b2/3, a value of /4 means a7/4 b/4, a value of /2 means a3/2 b/2. Default is /3. 
- crX(start,end,duration).: slide the specified value from start to end across duration. X is to be replaced with the value that has to be slide: d=last RPN/NRPN, h=pitch bend, n=panning, u=channel pressure, v=channel volume, w=modulation wheel, x=expression
- crX(param,start,end,duration).: some slides need an additional parameter. c=custom CC(param=CC number), k=key polypressure/aftertouch (param=key note)
- ctrl(cc,value).: custom control change (CC number 0-127 or CC name, value 0-127)
- controllerName(value)): control change, where controllerName is a CC name (see below), and value 0-127
- slide(controllerName, start, end, duration): slide the specified value from start to end across duration. ControllerName is a CC number or name (see below), or one of pitch, pressure or aftertouch.
- echo(time,volume,count,dest,octave).: following notes are echoed. time=length of the echo (a duration specifier like /2), volume=volume of the echo in percentages (1-99), count=optional number of echoes (1-99, default=3), dest=optional destination channel (0-15, default=current), octave=optional relative octave change (-10-10, default=0)
- echo:off.: turn a preceeding echo setting off
- meta(type,values...).: Send a custom MIDI meta message. type=0-127, values=double quoted strings (encoded in UTF-8) or numbers in one of the forms 123 (single byte), 123S (short), 123L (int), 3.14f (float), 3.14d (double) or 123J (64 bit int)
- note(key,velocity,duration): play specified note (this command can be useful for scripts). Velocity is optional and takes the current channel velocity if omited.
- nrpn(controllerMSB,controllerLSB,valueMSB,valueLSB).: NRPN control change (see MIDI spec)
- onnoteon{...}: a serie of command that have to be executed each time a note is played. IN the enclosed code, variables $note, $velocity, $duration and $channel are defined. Be aware that there is no protection against infinite loops, so don't put note commands in there !
- pressuredest(t,x).: channel pressure destination effect (see GM2 doc). t=type (pitch|volume|vibrato|filter), x=value (0-127)
- rpn(controllerMSB,controllerLSB,valueMSB,valueLSB).: RPN control change (see MIDI spec)
- sysex(values...).: send a custom system exclusive message. values=double quoted strings (encoded in UTF-8) or numbers in one of the forms 123 (single byte), 123S (short), 123L (int), 3.14f (float), 3.14d (double) or 123J (64 bit int). Terminating 0xF7 is automatically added.
- transpose(n).: transpose all notes except drums; n=semitones (+-108)
- velpattern(values..., other, beat) pattern of velocity to apply to notes depending on their beat and bar position. Values are expressed in percentages of full velocity. No velpattern is defined by default.
If a note falls on a beat, the position in the bar gives the value to take. If the note falls outside of a beat, the other (last value) is taken.
Example: for the notes abc/d/ef2g2 and velpattern(100, 90, 85, 80, 75, 1), the velocity of a is 100% (beat 1/4), b 90% (beat 2/4), c 85% (beat 3/4), d 75% (beat 3.5/4), e 80% (beat 4/4), f 100% (beat 1/4) and g 85% (beat 3/4).

CC names for ctrl, crc, slide,  and controlName(value) commands:

- bank, bankmsb (0)
- vibrato, modulation, vib, mod  (1)
- breath (2)
- foot (4)
- portatime, portamentotime  (5)
- data, datamsb (6)
- volume, vol (7)
- balance (8)
- pan, panning  (10)
- expression, expr  (11)
- banklsb (32)
- datalsb (38)
- sustain, pedal, hold, hold1 (64)
- portamento, porta (65)
- sostenuto (66)
- soft (67)
- legato (68)
- hold2 (69)
- resonnance, filterq, timbre (71)
- release (72)
- attack (73)
- brightness, filtercutoff (74)
- decay (75)
- vibratorate, vibratospeed, vibspeed  (76)
- vibratodepth, vibdepth  (77)
- vibratodelay, vibdelay (78)
- portanote, portamentonote (84)
- reverb (91)
- tremolo (92)
- chorus (93)
- userfx, detune (94)
- phaser (95)
- dataincrement, datainc (96)
- datadecrement, datadec (97)
- nrpnlsb (98)
- nrpnmsb (99)
- rpnlsb (100)
- rpnmsb (101)
- allsoundsoff (120)
- reset (121)
- allnotesoff (123)
- monophonic (126)
- polyphonic (127)

# Configuration MidiText.ini
- bassUpdatePeriod=ms: BASS Update period, default: 20ms. Shorter update period means more CPU usage.
- bassBufferLength=ms: BASS buffer length, default: 60ms. Shorter buffer means more responsiveness, but more CPU usage; longer buffer means more lag, but less CPU usage. Must be approx. at least 1.5 times greater than bassUpdatePeriod.
- fontX=file, name: load a soundfont, where X is font index, file is SF2/SF3/SFZ file, name is a name to use in mappings
- fontmapX=name, srcBank, srcPreset, dstBank, dstPreset, dstBankLSB: create a soundfont mapping: name=name of the font (not filename), srcBank/Preset=banks/presets to use from the soundfont (-1=all), dstBank/Preset/BankLSB=Bank and preset on which selected instruments are mapped in MIDI files (if srcBank/Preset are set to -1, dstPreset should be also set to -1 and dstBank specifies a bank offset). If all parameters are omited, defaults are -1 -1 0 -1 0.

# Keyboard shortcuts
## General keyboard shortcuts - always available
- Ctrl+A: Select all
- Ctrl+F: Find
- Ctrl+H: Search/replace
- Ctrl+I: select instrument
- Ctrl+S: save
- Ctrl+Shift+S: save as
- F3: Find next
- Shift+F3: Find previous
- F5: Play from cursor
- F6: Seek 5 seconds backwards (also works in open dialog box)
- F7: Seek 5 seconds forward (also works in open dialog box)
- F8: Stop playback (also works in open dialog box)
- - F9: Start/stop record (not yet implemented)
- - Shift+F9: enter/leave synthesizer mode without record (not yet implemented)
- Ctrl+F9: Select MIDI input device (not yet implemented)
- F10: Start/stop metronome (not yet implemented)
- Ctrl+F10: Metronome settings (not yet implemented)
- F11: Decrease volume (also works in open dialog box)
- F12: Increase volume (also works in open dialog box)

## Keyboard shortcuts in synthesizer mode when not using MIDI input device / when using the PC keyboard (not yet implemented)
- Up/down arrow: change octave up/down
- Left/right arrow: accelerate/slow down tempo/metronome
- `\zxcvbnm,./`	1st octave notes
- `asghjl;'\`	sharp notes for 1st octave
- `qwertyuiop[]`	2nd octave notes
- `2356790=`	sharp notes for 2nd octave
