local function convOctave (n)
return 'o' .. (n+5)
end

local function convEcho (dur, prc, cnt, ch)
if dur:sub(1,1)=='/' then dur = '1'..dur end
if cnt=='off' then cnt=2 end
return string.format("echo:%s,%s,%s,%d", dur, prc, cnt, ch+1)
end

local function convVoice (n) 
return 'V:' .. (n+1)
end

local function convProgram (bank, program)
local banklsb = 0
bank = bank+0
print(bank, program)
if bank>=128 then 
banklsb = bank%128
bank = bank//128 
end
return string.format("p(%d, %d, %d)", bank, banklsb, program)
end

local function convPitchRange (n)
return 'H' .. (n*100)
end

local function convRepeat (base, first, last, count)
return '|:' .. base .. '|1' .. first .. ':|' .. count .. '||'
end

local rules = {
{ '%f[%S]r(%d*/?%d*%*?)%f[%s]', 'R%1' },
{ '%f[%S](-?%d*)([a-gA-G])b', '%1%2_' },
{ '%((.-)%|(.-)%|(.-)%)(%d+)', convRepeat },
{ 'keyall:(-?%d+)', 'transpose(%1)' },
{ 'oct(-?%d+)', convOctave },
{ 'maxnotelength:off', 'm0' },
{ 'maxnotelength:(%S+)', 'm%1' },
{ 'fixecho:(.-),(.-),(.-),(%d+)', convEcho },
{ 'fixecho:(%S+)', 'echo:%1' },
{ 'vibrato(%d+)', 'w%1' },
{ 'vc(%d+)', 'V%1' },
{ '%[[vV]:(%d+)%]', convVoice },
{ 'p(%d+),(%d+)', convProgram },
{ 'l(%d+)', 'n%1' },
{ 'h.sens(%d+)', convPitchRange },
{ 'tem(%d+)', 't%1' },
{ 'k:on', 'k' },
{ 'k:off', 'K' },
{ 'k:sustain', '' },
{ 'h.quant?%d+', '' },
{ 'effectqua[ln]:?%d+', '' },
{ 'mult:(%S+)', 'L%1' },
{ 'resolution:%d+', '' },
{ 'crp:', 'cru:' },
{ 'chp(%d+)', 'u%1' }, 
{ '(%S+=%S+)', '$%1' },
{ '%%(%w+)%%', '$%1' },
{ '(onnoteon:%S+)', '//%1' },
{ '#restype:%S+', '' }
}

function convertFromV3 (code) 
for i, rule in ipairs(rules) do
code = code:gsub(rule[1], rule[2])
end
return code
end
