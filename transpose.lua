local xnotes = {
C=0, D=2, E=4, F=5, G=7, A=9, B=11,
['C#']=1, ['D#']=3, ['F#']=6, ['G#']=8, ['A#']=10,
D_=1, E_=3, G_=6, A_=8, B_=10,
}

local notes = {}
for s,v in pairs(xnotes) do 
notes[v]=s
notes[v+12]=s:lower()
notes[s]=v
notes[s:lower()]=v+12
end

function notevalue (note, alt)
if alt=='+' then alt='#'
elseif alt=='-' then alt='_'
end
local re = notes[note..alt]
return re
end

function valuenote (note, oct)
if not oct then oct='' end
while note>=24 do 
oct=oct.."'"
note = note -12
end
while note<0 do
oct = oct..','
note = note +12
end
local re = notes[note]..oct
return re
end

function transposeUp (note, alt, oct)
local note = notevalue(note,alt)
return valuenote(note+1, oct)
end

function transposeDown (note, alt, oct)
local note = notevalue(note,alt)
return valuenote(note -1, oct)
end

function transposeHelper (n)
if n==-1 then return transposeDown
elseif n==1 then return transposeUp
else error('Unknown transposition '..n)
end end

function transpose (s,n)
return s:gsub("([abcdefgABCDEFG])([-+#_]?)([,']*)", transposeHelper(n))
end

print('transpose OK')