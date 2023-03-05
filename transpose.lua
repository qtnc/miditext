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
print('Note>value: ', note..alt, re)
return re
end

function valuenote (oct, note)
if not oct or oct=='' then oct=0 end
while note>=24 do 
oct=oct+1
note = note -12
end
while note<0 do
oct = oct -1
note = note +12
end
if oct==0 then oct='' end
local re = oct..notes[note]
print('Value>note: ', oct, note, re)
return re
end

function transposeUp (oct, note, alt)
local note = notevalue(note,alt)
return valuenote(oct, note+1)
end

function transposeDown (oct, note, alt)
local note = notevalue(note,alt)
return valuenote(oct, note-1)
end

function transposeHelper (n)
if n==-1 then return transposeDown
elseif n==1 then return transposeUp
else print('Problem1')
end end

function transpose (s,n)
return s:gsub("(%-?%d?)([abcdefgABCDEFG])([-+#_,']?)", transposeHelper(n))
end

print('transpose OK')