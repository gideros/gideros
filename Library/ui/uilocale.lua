--!NEEDS:uiinit.lua

local LANG=application:getLanguage():lower()
if JS then LANG=JS.eval("navigator.language || navigator.userLanguage"):lower() end
local LANGS=LANG:sub(1,2)
local S={}

S.en={
	DAY1="M",
	DAY2="T",
	DAY3="W",
	DAY4="T",
	DAY5="F",
	DAY6="S",
	DAY7="S",
	MONTH1="January",
	MONTH2="February",
	MONTH3="March",
	MONTH4="April",
	MONTH5="May",
	MONTH6="June",
	MONTH7="July",
	MONTH8="August",
	MONTH9="September",
	MONTH10="October",
	MONTH11="November",
	MONTH12="December",
	["UI.DatePicker.Format"]="MM/DD/YYYY",
	["UI.DatePicker.IFormat"]="%m/%d/%Y",
	["UI.TimePicker.Format"]="HH:MM",
	["UI.TimePicker.IFormat"]="%H:%M",
}

S.fr={
	DAY1="L",
	DAY2="M",
	DAY3="M",
	DAY4="J",
	DAY5="V",
	DAY6="S",
	DAY7="D",
	MONTH1="Janvier",
	MONTH2="Février",
	MONTH3="Mars",
	MONTH4="Avril",
	MONTH5="Mai",
	MONTH6="Juin",
	MONTH7="Juillet",
	MONTH8="Août",
	MONTH9="Septembre",
	MONTH10="Octobre",
	MONTH11="Novembre",
	MONTH12="Décembre",
	["UI.DatePicker.Format"]="JJ/MM/AAAA",
	["UI.DatePicker.IFormat"]="%d/%m/%Y",
}
function UI._LO(t,s)
	return ((s or S)[LANG] and (s or S)[LANG][t]) or ((s or S)[LANGS] and (s or S)[LANGS][t]) or (s or S)['en'][t] or t
end