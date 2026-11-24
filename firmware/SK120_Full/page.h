#pragma once
// Web UI for XY-SK120 full controller (Arabic / English). Served from flash (PROGMEM).
const char PAGE_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="ar" dir="rtl">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>XY-SK120 Control</title>
<style>
:root{--bg:#020617;--card:#0b1222;--line:#1e293b;--mut:#94a3b8;--txt:#e5e7eb;--acc:#2563eb;--ok:#22c55e;--bad:#ef4444;--v:#22d3ee;--i:#a3e635;--p:#fbbf24}
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,-apple-system,'Segoe UI',Tahoma,sans-serif;background:var(--bg);color:var(--txt);font-size:14px}
.app{max-width:540px;margin:0 auto;padding:10px 12px 40px}
header{display:flex;justify-content:space-between;align-items:center;gap:8px;margin:4px 0 10px}
h1{font-size:17px;flex:1}h2{font-size:14px;margin:2px 0 8px}
.pill{font-size:11px;padding:3px 9px;border-radius:999px;background:#0f172a;color:var(--mut);border:0}
.pill.ok{background:#052e16;color:#86efac}.pill.bad{background:#450a0a;color:#fca5a5}
#lang{cursor:pointer;font-size:12px;padding:5px 10px;background:#1e293b;color:var(--txt)}
.tabs{display:flex;gap:2px;background:#0a1020;border-radius:12px;padding:3px;margin-bottom:10px;overflow-x:auto;scrollbar-width:none}
.tabs button{flex:1;min-width:62px;border:0;background:none;color:var(--mut);font-size:12px;padding:8px 4px;border-radius:9px;cursor:pointer;white-space:nowrap}
.tabs button.on{background:var(--acc);color:#fff}
.card{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:12px;margin-bottom:10px}
.g2{display:grid;grid-template-columns:1fr 1fr;gap:8px}.g3{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}
.m{background:var(--bg);border:1px solid var(--line);border-radius:12px;padding:8px 10px}
.m .l{font-size:11px;color:var(--mut)}.m .v{font-size:24px;font-weight:700;font-variant-numeric:tabular-nums;direction:ltr;text-align:start}
[dir=rtl] .m .v{text-align:end}
.m .v small{font-size:12px;font-weight:500;margin-left:3px;color:var(--mut)}
.m.s .v{font-size:16px}
.vV{color:var(--v)}.vI{color:var(--i)}.vP{color:var(--p)}
.badge{display:inline-block;font-size:11px;font-weight:700;padding:2px 8px;border-radius:6px;background:#1e293b}
.badge.cv{background:#083344;color:var(--v)}.badge.cc{background:#3f1d0b;color:#fdba74}
.alert{display:none;background:#450a0a;border:1px solid var(--bad);border-radius:12px;padding:10px;margin-bottom:10px}
.alert b{color:#fca5a5}
.row{display:flex;gap:8px;align-items:center}
.set{margin:8px 0}
.set .top{display:flex;justify-content:space-between;align-items:center;color:var(--mut);font-size:12px;margin-bottom:3px}
.set input[type=number]{width:110px;font-size:18px;font-weight:700}
input[type=range]{width:100%;accent-color:#38bdf8}
input,select{background:var(--bg);color:var(--txt);border:1px solid var(--line);border-radius:8px;padding:7px 8px;font-size:13px;width:100%}
input[type=number]{direction:ltr;text-align:right}
button.b{border:0;border-radius:10px;padding:10px 12px;font-weight:600;font-size:13px;cursor:pointer;background:#1e293b;color:var(--txt)}
button.b.pri{background:var(--acc);color:#fff}button.b.ok{background:var(--ok);color:#052e16}button.b.bad{background:var(--bad);color:#fff}
button.b:disabled{opacity:.4;cursor:not-allowed}
a.b{display:block;border-radius:10px;padding:10px 12px;font-weight:600;font-size:13px;background:#1e293b;color:var(--txt);text-align:center;text-decoration:none;flex:1}
.big{flex:1;padding:14px;font-size:15px}
.chips{display:grid;grid-template-columns:repeat(5,1fr);gap:6px}
.chips button{border-radius:9px;padding:8px 2px;background:var(--bg);border:1px solid var(--line);color:var(--txt);font-size:12px;cursor:pointer;line-height:1.25;direction:ltr}
.chips button b{display:block;font-size:13px}
.chips button:disabled{opacity:.4}
.f{display:grid;grid-template-columns:1fr 130px;gap:6px 10px;align-items:center}
.f label{font-size:13px}.f label i{display:block;font-style:normal;font-size:11px;color:var(--mut)}
.hint{font-size:11px;color:var(--mut);margin-top:6px;line-height:1.6}
canvas{width:100%;height:150px;display:block}
.leg{font-size:11px;color:var(--mut);margin-top:4px}.leg span{margin-inline-end:10px}
table{width:100%;border-collapse:collapse;font-size:12px;font-variant-numeric:tabular-nums}
td,th{border-bottom:1px solid var(--line);padding:4px;text-align:start}
#toast{position:fixed;left:50%;bottom:18px;transform:translateX(-50%);background:#0f172a;border:1px solid var(--line);padding:9px 16px;border-radius:10px;font-size:13px;opacity:0;transition:.25s;pointer-events:none;max-width:90%}
#toast.show{opacity:1}#toast.err{border-color:var(--bad);color:#fca5a5}
.pg{display:none}.pg.on{display:block}
.sw{display:flex;justify-content:space-between;align-items:center;gap:8px;padding:6px 0}
.sw input,.sw select{width:120px;flex:none}
.mut{color:var(--mut);font-size:12px}
.ltr{direction:ltr;unicode-bidi:embed}
a{color:#93c5fd}
</style>
</head>
<body>
<div class="app">
<header><h1>XY-SK120 <span class="mut">WiFi</span></h1><span id="link" class="pill" data-t="connecting"></span><button id="lang" class="pill">EN</button></header>
<nav class="tabs" id="tabs">
<button data-p="live" class="on" data-t="tLive"></button><button data-p="prot" data-t="tProt"></button><button data-p="mem" data-t="tMem"></button><button data-p="modes" data-t="tModes"></button><button data-p="prog" data-t="tProg"></button><button data-p="sys" data-t="tSys"></button><button data-p="net" data-t="tNet"></button>
</nav>

<!-- ============ LIVE ============ -->
<section class="pg on" id="p-live">
<div class="alert" id="alert"><div class="row" style="justify-content:space-between"><div><b id="alTitle"></b><div class="hint" id="alText" style="color:#fecaca"></div></div><button class="b bad" onclick="clearProt()" data-t="clear"></button></div></div>
<div class="card">
 <div class="g2">
  <div class="m"><div class="l" data-t="outV"></div><div class="v vV"><span id="vo">0.00</span><small>V</small></div></div>
  <div class="m"><div class="l" data-t="outI"></div><div class="v vI"><span id="io">0.000</span><small>A</small></div></div>
  <div class="m"><div class="l" data-t="outP"></div><div class="v vP"><span id="po">0.00</span><small>W</small></div></div>
  <div class="m"><div class="l" data-t="modeOut"></div><div class="v" style="font-size:18px;padding-top:4px"><span id="cvcc" class="badge">--</span> <span id="onTxt" class="badge">OFF</span></div></div>
 </div>
 <div class="g3" style="margin-top:8px">
  <div class="m s"><div class="l" data-t="vin"></div><div class="v"><span id="vin">0.00</span><small>V</small></div><div class="l ltr" id="iinEst" style="margin-top:2px" title=""></div></div>
  <div class="m s"><div class="l" data-t="tin"></div><div class="v"><span id="tin">--</span><small class="tu">°C</small></div></div>
  <div class="m s"><div class="l" data-t="tex"></div><div class="v"><span id="tex">--</span><small class="tu">°C</small></div></div>
  <div class="m s"><div class="l" data-t="cap"></div><div class="v"><span id="ah">0.000</span><small>Ah</small></div></div>
  <div class="m s"><div class="l" data-t="energy"></div><div class="v"><span id="wh">0.000</span><small>Wh</small></div></div>
  <div class="m s"><div class="l" data-t="ontime"></div><div class="v"><span id="tm">0:00:00</span></div></div>
 </div>
</div>

<div class="card">
 <canvas id="gr" style="touch-action:none"></canvas>
 <div class="leg row" style="justify-content:space-between"><div><span style="color:var(--v)">■ V</span><span style="color:var(--i)">■ A</span><span style="color:var(--p)">■ W</span></div>
  <select id="gRange" style="width:auto;padding:3px 6px;font-size:11px"><option value="180">3 min</option><option value="600">10 min</option><option value="1800">30 min</option></select></div>
</div>

<div class="card" id="jobsCard">
 <div id="jobLine" class="row" style="flex-wrap:wrap;gap:6px;margin-bottom:8px"></div>
 <h2 data-t="timerT"></h2>
 <div class="chips" id="tmrChips"></div>
</div>

<div class="card">
 <div class="set"><div class="top"><span data-t="setV"></span><input type="number" id="vN" class="lk" step="0.01" min="0" max="36"></div>
  <input type="range" id="vS" class="lk" min="0" max="3600" dir="ltr"></div>
 <div class="set"><div class="top"><span data-t="setI"></span><input type="number" id="iN" class="lk" step="0.001" min="0" max="6"></div>
  <input type="range" id="iS" class="lk" min="0" max="6000" dir="ltr"></div>
 <div class="row" style="margin-top:10px">
  <button class="b big lk" id="pwr"></button>
  <button class="b big" id="lkb"></button>
 </div>
</div>

<div class="card">
 <h2 data-t="recallT"></h2>
 <div class="chips" id="recall"></div>
 <h2 style="margin-top:12px" data-t="quickT"></h2>
 <div class="chips" id="quick"></div>
</div>
</section>

<!-- ============ PROTECT ============ -->
<section class="pg" id="p-prot">
<div class="card">
 <h2 data-t="protT"></h2>
 <div class="f" id="fProt"></div>
 <div class="row" style="margin-top:12px"><button class="b" onclick="loadMem(act(),'fProt')" data-t="reload"></button><button class="b pri" style="flex:1" onclick="saveMem(act(),'fProt')" data-t="apply"></button></div>
 <div class="hint" data-t="protHint"></div>
</div>
</section>

<!-- ============ MEMORY ============ -->
<section class="pg" id="p-mem">
<div class="card">
 <div class="row"><select id="memSel" style="flex:1"></select><button class="b" onclick="loadMem(+memSel.value,'fMem')" data-t="read"></button></div>
 <div class="f" id="fMem" style="margin-top:10px"></div>
 <div class="row" style="margin-top:12px">
  <button class="b" onclick="copyActive()" data-t="copyM0"></button>
  <button class="b pri" style="flex:1" onclick="saveMem(+memSel.value,'fMem')" data-t="saveGrp"></button>
  <button class="b ok" onclick="recall(+memSel.value)" data-t="recall"></button>
 </div>
 <div class="hint" data-t="memHint"></div>
</div>
</section>

<!-- ============ MODES ============ -->
<section class="pg" id="p-modes">
<div id="noExt" class="card" style="display:none"><span class="mut" data-t="noExt"></span></div>
<div class="card" id="cpCard">
 <h2 data-t="cpT"></h2>
 <div class="sw"><span data-t="cpMode"></span><select id="cpSw"><option value="0">OFF</option><option value="1">ON</option></select></div>
 <div class="sw"><span data-t="cpPow"></span><input type="number" id="cpW" step="0.1" min="0" max="120"></div>
 <button class="b pri" style="width:100%;margin-top:6px" onclick="saveCP()" data-t="apply"></button>
 <div class="hint" data-t="cpHint"></div>
</div>
<div class="card" id="mpCard">
 <h2 data-t="mpT"></h2>
 <div class="sw"><span>MPPT</span><select id="mpSw"><option value="0">OFF</option><option value="1">ON</option></select></div>
 <div class="sw"><span data-t="mpK"></span><input type="number" id="mpK" step="0.01" min="0.5" max="1"></div>
 <button class="b pri" style="width:100%;margin-top:6px" onclick="saveMPPT()" data-t="apply"></button>
 <div class="hint" data-t="mpHint"></div>
</div>
<div class="card" id="bfCard">
 <h2 data-t="bfT"></h2>
 <div class="sw"><span data-t="bfL"></span><input type="number" id="bf" step="0.001" min="0" max="6"></div>
 <button class="b pri" style="width:100%;margin-top:6px" onclick="saveBF()" data-t="apply"></button>
 <div class="hint" data-t="bfHint"></div>
</div>
<div class="card" id="batCard">
 <h2 data-t="batT"></h2>
 <div class="sw"><span data-t="batType"></span><select id="bType" style="width:150px"><option value="li">Li-ion 4.20V</option><option value="lfp">LiFePO4 3.65V</option><option value="pb">Lead-acid 2.40V</option></select></div>
 <div class="sw"><span data-t="batCells"></span><input type="number" id="bCells" min="1" max="15" value="1"></div>
 <div class="sw"><span data-t="batCap"></span><input type="number" id="bCap" min="100" step="100" value="2000"></div>
 <div class="sw"><span data-t="batRate"></span><select id="bRate"><option value="0.2">0.2C</option><option value="0.5" selected>0.5C</option><option value="1">1C</option></select></div>
 <div class="hint ltr" id="bCalc" style="font-size:13px;color:var(--txt)"></div>
 <div class="row" style="margin-top:8px"><button class="b bad" onclick="stopCharge()" data-t="stop"></button><button class="b ok" style="flex:1" onclick="startCharge()" data-t="startChg"></button></div>
 <div id="bProg" style="margin-top:8px;font-weight:600"></div>
 <div class="hint" data-t="batHint"></div>
</div>
</section>

<!-- ============ PROGRAMS ============ -->
<section class="pg" id="p-prog">
<div class="card">
 <h2 data-t="softT"></h2>
 <div class="sw"><span data-t="softL"></span><input type="number" id="softS" step="0.5" min="0" max="60"></div>
 <button class="b pri" style="width:100%;margin-top:6px" onclick="saveSoft()" data-t="save"></button>
 <div class="hint" data-t="softHint"></div>
</div>
<div class="card">
 <h2 data-t="timerT2"></h2>
 <div class="sw"><span data-t="tmrL"></span><input type="number" id="tmrMin" min="1" step="1" value="10"></div>
 <div class="row" style="margin-top:6px"><button class="b" onclick="setTimer(0)" data-t="cancel"></button><button class="b pri" style="flex:1" onclick="setTimer(Math.round(+$('tmrMin').value*60))" data-t="start"></button></div>
</div>
<div class="card">
 <h2 data-t="schedT"></h2>
 <div class="sw"><span data-t="enable"></span><select id="schEn"><option value="0">OFF</option><option value="1">ON</option></select></div>
 <div class="sw"><span data-t="schOn"></span><input type="time" id="schOn"></div>
 <div class="sw"><span data-t="schOff"></span><input type="time" id="schOff"></div>
 <button class="b pri" style="width:100%;margin-top:6px" onclick="saveSched()" data-t="save"></button>
 <div class="hint" data-t="schedHint"></div>
</div>
<div class="card">
 <h2 data-t="stepsT"></h2>
 <table id="stepTbl" dir="ltr"></table>
 <div class="sw" style="margin-top:6px"><button class="b" onclick="addStep()" data-t="addStep"></button><span style="flex:1"></span><span data-t="repeat"></span><input type="number" id="stRep" min="0" max="255" value="0" style="width:80px"></div>
 <div class="row" style="margin-top:6px"><button class="b" onclick="saveSteps()" data-t="save"></button><button class="b ok" style="flex:1" id="progBtn" onclick="toggleProg()"></button></div>
 <div class="hint" data-t="stepsHint"></div>
</div>
</section>

<!-- ============ SYSTEM ============ -->
<section class="pg" id="p-sys">
<div class="card">
 <h2 data-t="devSet"></h2>
 <div class="sw"><span data-t="bl"></span><select id="sBl"><option>0</option><option>1</option><option>2</option><option>3</option><option>4</option><option>5</option></select></div>
 <div class="sw"><span data-t="sleep"></span><input type="number" id="sSl" min="0" max="60"></div>
 <div class="sw"><span data-t="buzz"></span><select id="sBz"><option value="0">OFF</option><option value="1">ON</option></select></div>
 <div class="sw"><span data-t="tunit"></span><select id="sFc"><option value="0">°C</option><option value="1">°F</option></select></div>
 <div class="sw"><span data-t="tOfsI"></span><input type="number" id="sTo" step="0.1"></div>
 <div class="sw"><span data-t="tOfsE"></span><input type="number" id="sTe" step="0.1"></div>
 <div class="sw"><span data-t="adL"></span><input type="number" id="adA" step="0.1" min="0.1" max="20"></div>
 <div class="row" style="margin-top:8px"><button class="b" onclick="fillSys()" data-t="reload"></button><button class="b pri" style="flex:1" onclick="saveSys()" data-t="apply"></button></div>
</div>
<div class="card">
 <h2 data-t="clockT"></h2>
 <div class="sw"><span data-t="clockEn"></span><select id="clkEn"><option value="0">OFF</option><option value="1">ON</option></select></div>
 <div class="sw"><span data-t="tzL"></span><input type="number" id="tzH" step="0.5" min="-12" max="14"></div>
 <div class="sw"><span data-t="devTime"></span><span id="devTime" class="ltr">--</span></div>
 <button class="b pri" style="width:100%;margin-top:6px" onclick="saveClock()" data-t="save"></button>
 <div class="hint" data-t="clockHint"></div>
 <div class="hint" id="clkNo" style="display:none;color:#fca5a5" data-t="clkNo"></div>
</div>
<div class="card">
 <h2 data-t="devInfo"></h2>
 <table id="devInfo"></table>
</div>
<div class="card">
 <h2 data-t="mbT"></h2>
 <div class="sw"><span data-t="baudL"></span><select id="bdSel"></select></div>
 <button class="b" style="width:100%" onclick="setBaud()" data-t="baudBtn"></button>
 <div class="sw" style="margin-top:6px"><span data-t="idL"></span><input type="number" id="mbId" min="1" max="247"></div>
 <button class="b" style="width:100%" onclick="setId()" data-t="idBtn"></button>
 <div class="hint" data-t="mbHint"></div>
</div>
<div class="card">
 <h2 data-t="rgT"></h2>
 <div class="row"><input id="rgA" data-ph="phAddr"><input id="rgN" data-ph="phCount" value="16" style="width:80px"><button class="b" onclick="rawRead()" data-t="read"></button></div>
 <div class="row" style="margin-top:6px"><input id="rgWA" data-ph="phAddr"><input id="rgWV" data-ph="phVal"><button class="b bad" onclick="rawWrite()" data-t="write"></button></div>
 <table id="rgOut" style="margin-top:8px" dir="ltr"></table>
</div>
<div class="card">
 <h2 data-t="fwT"></h2>
 <div class="row"><a class="b" href="/log.csv" data-t="dlLog"></a><a class="b" href="/update" data-t="fwUp"></a></div>
 <div class="hint" data-t="fwHint"></div>
 <button class="b bad" style="width:100%;margin-top:8px" onclick="if(confirm(t('rebootQ')))api('/reboot').then(()=>toast(t('rebooting')))" data-t="reboot"></button>
</div>
</section>

<!-- ============ NETWORK ============ -->
<section class="pg" id="p-net">
<div class="card">
 <h2 data-t="netSt"></h2>
 <table id="netInfo"></table>
</div>
<div class="card">
 <h2 data-t="staT"></h2>
 <div class="row"><select id="ssidList" style="flex:1"></select><button class="b" onclick="scanWifi()" data-t="scan"></button></div>
 <input type="password" id="staPass" data-ph="phPass" style="margin-top:6px">
 <button class="b ok" style="width:100%;margin-top:6px" onclick="connectSta()" data-t="connect"></button>
 <div class="hint" data-t="staHint"></div>
</div>
<div class="card">
 <h2 data-t="apT"></h2>
 <input id="apS" data-ph="phApS"><input id="apP" type="password" data-ph="phApP" style="margin-top:6px">
 <button class="b" style="width:100%;margin-top:6px" onclick="saveAp()" data-t="saveAp"></button>
</div>
<div class="card">
 <h2 data-t="tgT"></h2>
 <div class="sw"><span data-t="enable"></span><select id="tgEn"><option value="0">OFF</option><option value="1">ON</option></select></div>
 <input id="tgTok" data-ph="phTok" autocomplete="off"><input id="tgChat" data-ph="phChat" style="margin-top:6px">
 <div class="row" style="margin-top:6px"><button class="b" onclick="testTg()" data-t="test"></button><button class="b pri" style="flex:1" onclick="saveTg()" data-t="save"></button></div>
 <div class="hint" data-t="tgHint"></div><div class="hint" id="tgLast"></div>
</div>
<div class="card">
 <h2>MQTT / Home Assistant <span id="mqSt" class="pill"></span></h2>
 <div class="sw"><span data-t="enable"></span><select id="mqEn"><option value="0">OFF</option><option value="1">ON</option></select></div>
 <div class="sw"><span data-t="mqHost"></span><input id="mqHost" style="width:160px" placeholder="192.168.0.10"></div>
 <div class="sw"><span data-t="mqPort"></span><input type="number" id="mqPort" style="width:160px"></div>
 <div class="sw"><span data-t="mqUser"></span><input id="mqUser" style="width:160px" autocomplete="off"></div>
 <div class="sw"><span data-t="mqPass"></span><input id="mqPass" type="password" style="width:160px" autocomplete="new-password"></div>
 <div class="sw"><span data-t="mqBase"></span><input id="mqBase" style="width:160px"></div>
 <button class="b pri" style="width:100%;margin-top:6px" onclick="saveMq()" data-t="save"></button>
 <div class="hint" data-t="mqHint"></div>
</div>
</section>
</div>
<div id="toast"></div>

<script>
const $=id=>document.getElementById(id);
const L={
en:{connecting:'connecting…',online:'SK120 online',noReply:'SK120 no reply',wifiLost:'WiFi lost',
tLive:'Live',tProt:'Protect',tMem:'Memory',tModes:'Modes',tSys:'System',tNet:'Network',
clear:'Clear',outV:'Output Voltage',outI:'Output Current',outP:'Output Power',modeOut:'Mode / Output',vin:'Input (≈ estimated)',tin:'Temp int.',tex:'Temp ext.',cap:'Capacity',energy:'Energy',ontime:'On time',last3:'last 3 min',
setV:'Set Voltage (0–36 V)',setI:'Current Limit (0–6 A)',outOn:'Output ON',outOff:'Output OFF',locked:'🔒 Locked',unlocked:'🔓 Unlocked',
recallT:'Memory recall <span class="mut">M1–M9 → active</span>',quickT:'Quick presets',
protT:'Active protections (M{0})',reload:'Reload',apply:'Apply to device',protHint:'0 in OAH / OWH / OHP means the protection is disabled. OCP should be above the CC current limit, and OVP above the set voltage.',
read:'Read',write:'Write',copyM0:'Copy from active',saveGrp:'Save to group',recall:'Recall',memHint:'The group marked (active) is the one in use. Saving another group does not change the output until you Recall it.',active:'(active)',
noExt:'This SK120 firmware did not answer registers 0x1E–0x23, so CP / MPPT / battery modes are not available.',
cpT:'Constant Power (CP)',cpMode:'CP mode',cpPow:'Power (W)',cpHint:'Keeps a constant power in watts instead of constant voltage or current.',
mpT:'MPPT (solar input)',mpK:'K factor (Vmp/Voc)',mpHint:'When fed from a solar panel: keeps input voltage from dropping below K × open-circuit voltage (usually 0.80).',
bfT:'Battery charging cut-off',bfL:'Stop when current < (A)',bfHint:'Turns the output off when charge current in the CV stage falls below this value (0 = disabled).',
devSet:'Device settings',bl:'Backlight',sleep:'Sleep (min, 0 = never)',buzz:'Buzzer',tunit:'Temperature unit',tOfsI:'Int. temp offset',tOfsE:'Ext. temp offset',
devInfo:'Device info',model:'Model',skFw:'SK120 firmware',mbAddr:'Modbus address',baudDev:'Baud (device)',baudEsp:'Baud (ESP)',espFw:'ESP firmware',linkStat:'Link ok / err',
mbT:'Modbus link',baudL:'Baud rate (SK120 + ESP)',baudBtn:'Change baud on both',idL:'Slave ID used by ESP',idBtn:'Save ID',mbHint:'Higher baud = faster refresh. The ESP also auto-detects the baud if the link is lost.',
rgT:'Register tool',phAddr:'addr e.g. 0x00',phCount:'count',phVal:'value (raw)',
fwT:'Data & firmware',dlLog:'Download log (CSV)',fwUp:'Firmware update',fwHint:'Update login: <b>admin</b> / AP password. Upload the .bin from Arduino → Sketch → Export Compiled Binary.',reboot:'Reboot WiFi module',rebootQ:'Reboot ESP?',rebooting:'Rebooting…',
netSt:'Status',staT:'Home router (STA)',scan:'Scan',scanFirst:'— scan first —',scanning:'Scanning…',scanFail:'scan failed',phPass:'WiFi password',connect:'Connect & save',staHint:'The access point stays on as a fallback.',
apT:'Access point',phApS:'AP SSID',phApP:'AP password (8+ chars)',saveAp:'Save AP',
ap:'AP',router:'Router',notConn:'(not connected)',signal:'Signal',host:'Hostname',heap:'Free heap',uptime:'Uptime',
protTrip:'Protection tripped: ',writeFail:'Write failed: ',invalid:': invalid value',tooLarge:': too large',limVI:'V ≤ 36, I ≤ 6',
ovpLow:'OVP is lower than V-SET — output will trip. Continue?',ocpLow:'OCP is lower than I-SET. Continue?',readFail:'Read failed',savedTo:'Saved to M',saveFail:'Save failed: ',copied:'Copied active group — now Save',
devLocked:'Device locked',recalled:' recalled',recallFail:'Recall failed',setFail:'Set failed',pwrFail:'Output switch failed',lockFail:'Lock failed',cleared:'Protection cleared',
cpSaved:'CP saved',mpSaved:'MPPT saved',saved:'Saved',sysSaved:'Settings saved',baudQ:'Change Modbus baud to {0} on SK120 and ESP?',baudOk:'Baud changed',failed:'Failed',idSaved:'ID saved',
addrVal:'addr/value?',rawQ:'Write raw {0} to {1}?',written:'Written',pickNet:'Pick a network',connecting2:'Connecting… reopen via new IP or sk120.local',passLen:'Password ≥ 8 chars',apQ:'Change AP to "{0}"? You may need to reconnect.',apSaved:'AP saved',
tProg:'Programs',adL:'Input adapter max current (A)',adWarn:'Input adapter near its limit (≈{0}%) — reduce load or set OPP',estHint:'Estimated input (SK120 has no input-current sensor): output power ÷ 90% efficiency',min:'min',jobRamp:'Soft start…',jobTimer:'Off in {0}',jobProg:'Step {0}/{1} · {2}s left',jobChg:'Charging {0} mAh',
timerT:'Auto-off timer',cancel:'Cancel',start:'Start',stop:'Stop',save:'Save',enable:'Enabled',test:'Send test',
softT:'Soft start',softL:'Ramp time (s, 0 = off)',softHint:'When the output is switched on from this page (or by a program / schedule / MQTT), the voltage rises gradually from 0 to the set value. Reduces motor start-up current.',
timerT2:'Timer',tmrL:'Turn output off after (min)',timerSet:'Timer started',timerOff:'Timer cancelled',
schedT:'Daily schedule',schOn:'Output ON at',schOff:'Output OFF at',schedHint:'Uses the time zone from System → Screen clock. Leave a time empty to skip it.',
stepsT:'Step program',addStep:'+ Step',repeat:'Repeat (0 once, 255 forever)',runProg:'▶ Run program',stopProg:'■ Stop program',stepsHint:'Each step sets V and A for the given seconds. The program is stored in the module and keeps running with the page closed. Soft start applies to the first step.',maxSteps:'Max 10 steps',
batT:'Battery charger',batType:'Type',batCells:'Cells in series',batCap:'Capacity (mAh)',batRate:'Charge rate',startChg:'Start charging',
batCalc:'Charge {0} V · {1} A · stop below {2} A',batTooHigh:'Voltage above 36 V — use fewer cells',batQ:'Charge at {0} V / {1} A?\nCheck the battery is connected with correct polarity.',
batHint:'Writes V, I, OVP, OCP and OAH (capacity × 1.3 safety) to the active group plus the cut-off current, then turns the output on. You get a notification when it finishes.',
chgProg:'{0} mAh ({1}%) · {2} · {3}',chgStarted:'Charging started',
clockT:'Screen clock',clockEn:'Show clock on SK120 screensaver',tzL:'Time zone (hours from UTC)',devTime:'Module time',clockHint:'Time comes from the internet (NTP). Also used by the daily schedule.',clkNo:'This SK120 version does not accept the clock registers (0x0200). The screensaver clock needs the newer SK120X. Time is still used for the schedule.',
tgT:'Telegram alerts',phTok:'Bot token (from @BotFather)',phChat:'Chat ID',tgHint:'Alerts: protection tripped, battery charged, timer / program finished, schedule actions.',tgLastL:'Last send: ',sending:'Sending…',
mqHost:'Broker address',mqPort:'Port',mqUser:'User',mqPass:'Password',mqBase:'Topic prefix',mqOn:'connected',mqOff:'not connected',
mqHint:'Home Assistant finds the SK120 automatically (MQTT discovery). Control topics: &lt;prefix&gt;/set/voltage, /set/current, /set/output (ON/OFF). State: &lt;prefix&gt;/state.',
f:['Output voltage','Current limit (CC)','Low input voltage cut','Over-voltage cut','Over-current cut','Over-power cut','Run-time limit, hours','Run-time limit, minutes','Capacity limit','Energy limit','Over-temperature cut','Output ON at power-up'],
P:{OVP:'Output voltage exceeded the set limit',OCP:'Output current exceeded the set limit',OPP:'Output power exceeded the set limit',LVP:'Input voltage below the LVP limit',OAH:'Amp-hour limit reached',OHP:'Run-time limit reached',OTP:'Module over-temperature',OEP:'Output error',OWH:'Watt-hour limit reached',ICP:'Input current too high',ETP:'External sensor over-temperature'}},
ar:{connecting:'جاري الاتصال…',online:'SK120 متصل',noReply:'SK120 لا يرد',wifiLost:'انقطع الواي فاي',
tLive:'مباشر',tProt:'الحماية',tMem:'الذاكرة',tModes:'الأوضاع',tSys:'النظام',tNet:'الشبكة',
clear:'مسح',outV:'جهد الخرج',outI:'تيار الخرج',outP:'قدرة الخرج',modeOut:'الوضع / الخرج',vin:'الدخل (≈ تقديري)',tin:'حرارة داخلية',tex:'حرارة خارجية',cap:'السعة',energy:'الطاقة',ontime:'زمن التشغيل',last3:'آخر 3 دقائق',
setV:'ضبط الجهد <span class="ltr">(0–36 V)</span>',setI:'حد التيار <span class="ltr">(0–6 A)</span>',outOn:'الخرج يعمل',outOff:'الخرج مطفأ',locked:'🔒 مقفل',unlocked:'🔓 مفتوح',
recallT:'استدعاء الذاكرة <span class="mut">M1–M9 ← النشطة</span>',quickT:'إعدادات سريعة',
protT:'الحمايات النشطة (M{0})',reload:'تحديث',apply:'تطبيق على الجهاز',protHint:'القيمة 0 في OAH / OWH / OHP تعني أن الحماية معطّلة. يجب أن يكون OCP أعلى من حد التيار CC، و OVP أعلى من الجهد المضبوط.',
read:'قراءة',write:'كتابة',copyM0:'نسخ من النشطة',saveGrp:'حفظ في المجموعة',recall:'استدعاء',memHint:'المجموعة المعلّمة (النشطة) هي المستخدمة حالياً. حفظ مجموعة أخرى لا يغيّر الخرج حتى تستدعيها.',active:'(النشطة)',
noExt:'إصدار هذا الجهاز لم يرد على الريجسترات 0x1E–0x23، لذلك أوضاع CP / MPPT / البطارية غير متاحة.',
cpT:'القدرة الثابتة (CP)',cpMode:'وضع CP',cpPow:'القدرة (W)',cpHint:'يحافظ على قدرة ثابتة بالواط بدلاً من جهد أو تيار ثابت.',
mpT:'MPPT (دخل لوح شمسي)',mpK:'معامل K (Vmp/Voc)',mpHint:'عند التغذية من لوح شمسي: يمنع جهد الدخل من الهبوط تحت K × جهد الدائرة المفتوحة (عادة 0.80).',
bfT:'فصل شحن البطارية',bfL:'افصل عندما التيار أقل من (A)',bfHint:'يفصل الخرج عندما ينخفض تيار الشحن في مرحلة CV تحت هذه القيمة (0 = معطّل).',
devSet:'إعدادات الجهاز',bl:'إضاءة الشاشة',sleep:'السكون (دقيقة، 0 = أبداً)',buzz:'الصفارة',tunit:'وحدة الحرارة',tOfsI:'تعويض الحرارة الداخلية',tOfsE:'تعويض الحرارة الخارجية',
devInfo:'معلومات الجهاز',model:'الموديل',skFw:'إصدار SK120',mbAddr:'عنوان Modbus',baudDev:'السرعة (الجهاز)',baudEsp:'السرعة (ESP)',espFw:'إصدار ESP',linkStat:'نجاح / خطأ الاتصال',
mbT:'اتصال Modbus',baudL:'سرعة الاتصال (SK120 + ESP)',baudBtn:'تغيير السرعة على الاثنين',idL:'رقم الجهاز المستخدم',idBtn:'حفظ الرقم',mbHint:'سرعة أعلى = تحديث أسرع. الـ ESP يكتشف السرعة تلقائياً إذا انقطع الاتصال.',
rgT:'أداة الريجسترات',phAddr:'العنوان مثل 0x00',phCount:'العدد',phVal:'القيمة (خام)',
fwT:'البيانات والسوفت وير',dlLog:'تحميل السجل (CSV)',fwUp:'تحديث السوفت وير',fwHint:'دخول التحديث: <b>admin</b> / كلمة مرور الـ AP. ارفع ملف ‎.bin من Arduino ← Sketch ← Export Compiled Binary.',reboot:'إعادة تشغيل موديول الواي فاي',rebootQ:'إعادة تشغيل الـ ESP؟',rebooting:'جاري إعادة التشغيل…',
netSt:'الحالة',staT:'راوتر البيت (STA)',scan:'بحث',scanFirst:'— ابحث أولاً —',scanning:'جاري البحث…',scanFail:'فشل البحث',phPass:'كلمة مرور الواي فاي',connect:'اتصال وحفظ',staHint:'نقطة الوصول AP تبقى تعمل كاحتياط.',
apT:'نقطة الوصول (AP)',phApS:'اسم الشبكة AP',phApP:'كلمة مرور AP (8 أحرف أو أكثر)',saveAp:'حفظ AP',
ap:'نقطة الوصول',router:'الراوتر',notConn:'(غير متصل)',signal:'قوة الإشارة',host:'اسم الجهاز',heap:'الذاكرة الحرة',uptime:'مدة التشغيل',
protTrip:'اشتغلت حماية: ',writeFail:'فشلت الكتابة: ',invalid:': قيمة غير صحيحة',tooLarge:': قيمة كبيرة جداً',limVI:'الجهد ≤ 36 والتيار ≤ 6',
ovpLow:'OVP أقل من الجهد المضبوط — الخرج سيفصل. متابعة؟',ocpLow:'OCP أقل من حد التيار. متابعة؟',readFail:'فشلت القراءة',savedTo:'تم الحفظ في M',saveFail:'فشل الحفظ: ',copied:'تم نسخ المجموعة النشطة — اضغط حفظ',
devLocked:'الجهاز مقفل',recalled:' تم استدعاؤها',recallFail:'فشل الاستدعاء',setFail:'فشل الضبط',pwrFail:'فشل تشغيل/إطفاء الخرج',lockFail:'فشل القفل',cleared:'تم مسح الحماية',
cpSaved:'تم حفظ CP',mpSaved:'تم حفظ MPPT',saved:'تم الحفظ',sysSaved:'تم حفظ الإعدادات',baudQ:'تغيير سرعة Modbus إلى {0} على SK120 و ESP؟',baudOk:'تم تغيير السرعة',failed:'فشل',idSaved:'تم حفظ الرقم',
addrVal:'العنوان/القيمة؟',rawQ:'كتابة القيمة {0} في {1}؟',written:'تمت الكتابة',pickNet:'اختر شبكة',connecting2:'جاري الاتصال… افتح الصفحة من العنوان الجديد أو sk120.local',passLen:'كلمة المرور 8 أحرف على الأقل',apQ:'تغيير الـ AP إلى "{0}"؟ قد تحتاج لإعادة الاتصال.',apSaved:'تم حفظ AP',
tProg:'البرامج',adL:'أقصى تيار لمحوّل الدخل (A)',adWarn:'محوّل الدخل قريب من حده (≈{0}%) — خفّف الحمل أو اضبط OPP',estHint:'دخل تقديري (الجهاز لا يقيس تيار الدخل): قدرة الخرج ÷ كفاءة 90%',min:'د',jobRamp:'تشغيل ناعم…',jobTimer:'يُطفأ بعد {0}',jobProg:'خطوة {0}/{1} · باقي {2} ث',jobChg:'شحن {0} mAh',
timerT:'مؤقت الإطفاء',cancel:'إلغاء',start:'ابدأ',stop:'إيقاف',save:'حفظ',enable:'مفعّل',test:'رسالة تجربة',
softT:'التشغيل الناعم',softL:'زمن الصعود (ثانية، 0 = معطّل)',softHint:'عند تشغيل الخرج من هذه الصفحة (أو من برنامج / جدول / MQTT) يرتفع الجهد تدريجياً من 0 حتى القيمة المضبوطة. يقلل تيار بدء تشغيل المحركات.',
timerT2:'المؤقت',tmrL:'إطفاء الخرج بعد (دقيقة)',timerSet:'بدأ المؤقت',timerOff:'تم إلغاء المؤقت',
schedT:'جدول يومي',schOn:'تشغيل الخرج الساعة',schOff:'إطفاء الخرج الساعة',schedHint:'يستخدم المنطقة الزمنية من النظام ← ساعة الشاشة. اترك الوقت فارغاً لتجاهله.',
stepsT:'برنامج الخطوات',addStep:'+ خطوة',repeat:'تكرار (0 مرة، 255 دائماً)',runProg:'▶ تشغيل البرنامج',stopProg:'■ إيقاف البرنامج',stepsHint:'كل خطوة تضبط الجهد والتيار لعدد الثواني المحدد. البرنامج محفوظ في الموديول ويستمر حتى لو أغلقت الصفحة. التشغيل الناعم يطبَّق على الخطوة الأولى.',maxSteps:'الحد الأقصى 10 خطوات',
batT:'شاحن البطاريات',batType:'النوع',batCells:'عدد الخلايا (توالي)',batCap:'السعة (mAh)',batRate:'معدل الشحن',startChg:'ابدأ الشحن',
batCalc:'الشحن {0} V · {1} A · الفصل تحت {2} A',batTooHigh:'الجهد أعلى من 36 V — قلّل عدد الخلايا',batQ:'الشحن على {0} V / {1} A؟\nتأكد أن البطارية موصولة بالقطبية الصحيحة.',
batHint:'يكتب الجهد والتيار و OVP و OCP و OAH (السعة × 1.3 للأمان) في المجموعة النشطة مع تيار الفصل، ثم يشغّل الخرج. يصلك تنبيه عند الانتهاء.',
chgProg:'{0} mAh ({1}%) · {2} · {3}',chgStarted:'بدأ الشحن',
clockT:'ساعة الشاشة',clockEn:'عرض الساعة على شاشة السكون',tzL:'المنطقة الزمنية (ساعات عن UTC)',devTime:'وقت الموديول',clockHint:'الوقت يأتي من الإنترنت (NTP). ويُستخدم أيضاً في الجدول اليومي.',clkNo:'إصدار الـ SK120 هذا لا يقبل ريجسترات الساعة (0x0200). ساعة شاشة السكون تحتاج الإصدار الأحدث SK120X. الوقت ما زال مستخدماً في الجدول.',
tgT:'تنبيهات تيليجرام',phTok:'توكن البوت (من ‎@BotFather)',phChat:'رقم المحادثة Chat ID',tgHint:'التنبيهات: اشتغال حماية، اكتمال شحن البطارية، انتهاء المؤقت أو البرنامج، أحداث الجدول.',tgLastL:'آخر إرسال: ',sending:'جاري الإرسال…',
mqHost:'عنوان الـ Broker',mqPort:'المنفذ',mqUser:'المستخدم',mqPass:'كلمة المرور',mqBase:'بادئة المواضيع',mqOn:'متصل',mqOff:'غير متصل',
mqHint:'Home Assistant يكتشف الـ SK120 تلقائياً (MQTT discovery). مواضيع التحكم: &lt;prefix&gt;/set/voltage و /set/current و /set/output (ON/OFF). الحالة: &lt;prefix&gt;/state.',
f:['جهد الخرج','حد التيار (CC)','فصل عند انخفاض جهد الدخل','فصل عند زيادة الجهد','فصل عند زيادة التيار','فصل عند زيادة القدرة','حد زمن التشغيل، ساعات','حد زمن التشغيل، دقائق','حد السعة','حد الطاقة','فصل عند ارتفاع الحرارة','تشغيل الخرج عند بدء التشغيل'],
P:{OVP:'تجاوز جهد الخرج الحد المضبوط',OCP:'تجاوز تيار الخرج الحد المضبوط',OPP:'تجاوز القدرة (واط) الحد المضبوط',LVP:'جهد الدخل أقل من حد LVP',OAH:'تم الوصول لحد الأمبير-ساعة',OHP:'انتهى زمن التشغيل المحدد',OTP:'ارتفاع حرارة الوحدة',OEP:'خطأ في الخرج',OWH:'تم الوصول لحد الواط-ساعة',ICP:'تيار الدخل مرتفع',ETP:'ارتفاع حرارة الحساس الخارجي'}}};
let lang='ar';try{lang=localStorage.getItem('lang')||'ar'}catch(e){}
const t=(k,...a)=>{let s=(L[lang][k]??L.en[k]??k);a.forEach((v,i)=>s=s.replace('{'+i+'}',v));return s};

const PROT=['OK','OVP','OCP','OPP','LVP','OAH','OHP','OTP','OEP','OWH','ICP','ETP'];
const BAUDS=[9600,14400,19200,38400,56000,57600,115200,2400,4800];
const MF=[
 {l:'V-SET',u:'V',i:0,s:100,st:.01},{l:'I-SET',u:'A',i:1,s:1000,st:.001},{l:'LVP',u:'V',i:2,s:100,st:.01},{l:'OVP',u:'V',i:3,s:100,st:.01},
 {l:'OCP',u:'A',i:4,s:1000,st:.001},{l:'OPP',u:'W',i:5,s:10,st:.1},{l:'OHP h',u:'h',i:6,s:1,st:1},{l:'OHP m',u:'min',i:7,s:1,st:1},
 {l:'OAH',u:'Ah',i:8,s:1000,st:.001,w:1},{l:'OWH',u:'Wh',i:10,s:1000,st:.001,w:1},{l:'OTP',u:'°',i:12,s:1,st:1},{l:'POn',i:13,s:1,sel:1}];
let R=[],locked=false,on=false,editUntil=0,sysFilled=false,info={},lastOk=null;

function toast(m,e){const x=$('toast');x.textContent=m;x.className='show'+(e?' err':'');clearTimeout(x._h);x._h=setTimeout(()=>x.className='',2400)}
async function api(u){const r=await fetch(u);const s=await r.text();if(!r.ok)throw new Error(s||r.status);return s}
async function wr(a,v){try{await api('/w?a='+a+'&v='+Math.round(v));return true}catch(e){toast(t('writeFail')+'0x'+a.toString(16)+' '+e.message,1);return false}}
const s16=x=>x>32767?x-65536:x, pad=n=>String(n).padStart(2,'0'), u32=(lo,hi)=>hi*65536+lo;

// ---------- active memory group (register 0x1D) ----------
let shownAct=-1;
const act=()=>(R.length&&R[0x1D]<=9)?R[0x1D]:0;
function refreshAct(){const a=act();shownAct=a;
 let o='';for(let n=0;n<=9;n++)o+='<option value="'+n+'">M'+n+(n===a?' '+t('active'):'')+'</option>';const ms=$('memSel'),mv=ms.value||1;ms.innerHTML=o;ms.value=mv;
 document.querySelectorAll('[data-t=protT]').forEach(e=>e.innerHTML=t('protT',a))}

// ---------- language ----------
function applyLang(){const h=document.documentElement;h.lang=lang;h.dir=lang==='ar'?'rtl':'ltr';
 $('lang').textContent=lang==='ar'?'EN':'عربي';
 document.querySelectorAll('[data-t]').forEach(e=>e.innerHTML=t(e.dataset.t));
 document.querySelectorAll('[data-ph]').forEach(e=>e.placeholder=t(e.dataset.ph));
 refreshAct();
 if(!$('ssidList').options.length||!$('ssidList').value)$('ssidList').innerHTML='<option value="">'+t('scanFirst')+'</option>';
 ['fProt','fMem'].forEach(id=>{const keep=formVals(id);buildForm(id);keep.forEach((v,k)=>$(id+'_'+k).value=v)});
 document.querySelectorAll('.tmin').forEach(e=>e.textContent=t('min'));
 applyLock();if(R.length)render(lastOk);loadInfo();if(typeof batCalc==='function')batCalc()}
$('lang').onclick=()=>{lang=lang==='ar'?'en':'ar';try{localStorage.setItem('lang',lang)}catch(e){}applyLang()};

// ---------- tabs ----------
$('tabs').onclick=e=>{const p=e.target.dataset.p;if(!p)return;
 document.querySelectorAll('.tabs button').forEach(b=>b.classList.toggle('on',b===e.target));
 document.querySelectorAll('.pg').forEach(s=>s.classList.toggle('on',s.id==='p-'+p));
 if(p==='prot')loadMem(act(),'fProt');if(p==='sys'){fillSys();loadInfo();loadOpts()}if(p==='net'){loadInfo();loadOpts()}if(p==='prog')loadOpts();if(p==='modes')fillModes();if(p==='mem')loadMem(+$('memSel').value,'fMem')};

// ---------- memory forms ----------
function formVals(id){return MF.map((m,k)=>{const e=$(id+'_'+k);return e?e.value:''})}
function buildForm(id){const f=$(id);f.innerHTML='';MF.forEach((m,k)=>{
 const lb=document.createElement('label');lb.innerHTML='<span class="ltr">'+m.l+'</span>'+(m.u?' <span class="mut">('+m.u+')</span>':'')+'<i>'+t('f')[k]+'</i>';
 let inp;if(m.sel){inp=document.createElement('select');inp.innerHTML='<option value="0">OFF</option><option value="1">ON</option>'}
 else{inp=document.createElement('input');inp.type='number';inp.step=m.st;inp.min=0}
 inp.id=id+'_'+k;f.append(lb,inp)})}
function fillForm(id,r){MF.forEach((m,k)=>{const raw=m.w?u32(r[m.i],r[m.i+1]):r[m.i];$(id+'_'+k).value=m.sel?raw:+(raw/m.s).toFixed(m.s>=1000?3:m.s>=100?2:m.s>=10?1:0)})}
function readForm(id){const out=new Array(14).fill(0);for(let k=0;k<MF.length;k++){const m=MF[k];const v=parseFloat($(id+'_'+k).value);
 if(!isFinite(v)||v<0){toast(m.l+t('invalid'),1);return null}const raw=Math.round(v*m.s);
 if(m.w){out[m.i]=raw%65536;out[m.i+1]=Math.floor(raw/65536)}else{if(raw>65535){toast(m.l+t('tooLarge'),1);return null}out[m.i]=raw}}
 if(out[0]>3600||out[1]>6000){toast(t('limVI'),1);return null}
 if(out[3]&&out[3]<out[0]&&!confirm(t('ovpLow')))return null;
 if(out[4]&&out[4]<out[1]&&!confirm(t('ocpLow')))return null;
 return out}
async function loadMem(n,id){try{fillForm(id,JSON.parse(await api('/mem?n='+n)))}catch(e){toast(t('readFail')+' M'+n,1)}}
async function saveMem(n,id){const v=readForm(id);if(!v)return;
 try{await api('/wm?a='+(0x50+n*16)+'&v='+v.join(','));toast(t('savedTo')+n);if(n===0)editUntil=0}catch(e){toast(t('saveFail')+e.message,1)}}
async function copyActive(){try{fillForm('fMem',JSON.parse(await api('/mem?n='+act())));toast(t('copied'))}catch(e){toast(t('readFail'),1)}}
async function recall(n){if(locked)return toast(t('devLocked'),1);try{await api('/recall?n='+n);toast('M'+n+t('recalled'))}catch(e){toast(t('recallFail'),1)}}

// ---------- live controls ----------
const vS=$('vS'),iS=$('iS'),vN=$('vN'),iN=$('iN');
function hold(){editUntil=Date.now()+1500}
vS.oninput=()=>{vN.value=(vS.value/100).toFixed(2);hold()};iS.oninput=()=>{iN.value=(iS.value/1000).toFixed(3);hold()};
vN.oninput=iN.oninput=hold;
vS.onchange=iS.onchange=()=>sendSet();
vN.onchange=()=>{vS.value=Math.round(vN.value*100);sendSet()};iN.onchange=()=>{iS.value=Math.round(iN.value*1000);sendSet()};
vN.onkeydown=iN.onkeydown=e=>{if(e.key==='Enter')e.target.blur()};
async function sendSet(v,i){if(locked)return;v=v??vS.value/100;i=i??iS.value/1000;hold();
 try{await api('/set?v='+(+v).toFixed(2)+'&i='+(+i).toFixed(3))}catch(e){toast(t('setFail'),1)}}
function quick(v,i){vS.value=v*100;iS.value=i*1000;vN.value=v.toFixed(2);iN.value=i.toFixed(3);sendSet(v,i)}
$('pwr').onclick=async()=>{if(locked)return;try{await api('/power?on='+(on?0:1))}catch(e){toast(t('pwrFail'),1)}};
$('lkb').onclick=async()=>{try{await api('/lock?on='+(locked?0:1));locked=!locked;applyLock()}catch(e){toast(t('lockFail'),1)}};
async function clearProt(){await wr(0x10,0)&&toast(t('cleared'))}
function applyLock(){$('lkb').textContent=t(locked?'locked':'unlocked');$('lkb').className='b big'+(locked?' bad':'');
 document.querySelectorAll('.lk,#recall button,#quick button').forEach(e=>e.disabled=locked)}
function paintPower(){$('pwr').textContent=t(on?'outOn':'outOff');$('pwr').className='b big lk '+(on?'ok':'bad')}

(function(){let h='';for(let n=1;n<=9;n++)h+='<button onclick="recall('+n+')"><b>M'+n+'</b>⟲</button>';$('recall').innerHTML=h;
 const Q=[[3.3,.15],[5,.15],[9,.15],[12,.15],[24,.15],[3.3,.5],[5,.5],[9,.5],[12,.5],[24,.5],[3.3,1],[5,1],[9,1],[12,1],[24,1]];
 $('quick').innerHTML=Q.map(q=>'<button onclick="quick('+q[0]+','+q[1]+')"><b>'+q[0]+'V</b>'+q[1].toFixed(2)+'A</button>').join('');
 $('bdSel').innerHTML=BAUDS.map((b,i)=>'<option value="'+i+'">'+b+'</option>').join('');
 $('memSel').onchange=()=>loadMem(+$('memSel').value,'fMem')})();

// ---------- graph ----------
const cv=$('gr'),cx=cv.getContext('2d');let G={v:[],i:[],step:1,range:180,last:0},hov=-1;
try{G.range=+(localStorage.getItem('gRange')||180)}catch(e){}$('gRange').value=G.range;
const gCap=()=>Math.ceil(G.range/G.step);
async function loadHist(){try{const d=JSON.parse(await api('/log.json?sec='+G.range+'&pts=300'));G.v=d.v.map(x=>x/100);G.i=d.i.map(x=>x/1000);G.step=d.step;G.last=Date.now();draw()}catch(e){}}
function addLive(vo,io){if(Date.now()-G.last<G.step*1000-50)return;G.last=Date.now();G.v.push(vo);G.i.push(io);const c=gCap();while(G.v.length>c){G.v.shift();G.i.shift()}draw()}
function draw(){const d=devicePixelRatio||1,w=cv.clientWidth,h=cv.clientHeight;if(!w)return;if(cv.width!==w*d){cv.width=w*d;cv.height=h*d}
 cx.setTransform(d,0,0,d,0,0);cx.clearRect(0,0,w,h);cx.strokeStyle='#1e293b';cx.lineWidth=1;
 for(let k=1;k<4;k++){const y=h*k/4;cx.beginPath();cx.moveTo(0,y);cx.lineTo(w,y);cx.stroke()}
 const N=gCap(),n=G.v.length,X=k=>w*(k+N-n)/Math.max(1,N-1),P=G.v.map((v,k)=>v*G.i[k]);
 const line=(a,c)=>{if(a.length<2)return;const mx=Math.max(...a)*1.15||1;cx.strokeStyle=c;cx.lineWidth=1.8;cx.beginPath();
  a.forEach((v,k)=>{const x=X(k),y=h-4-(h-8)*v/mx;k?cx.lineTo(x,y):cx.moveTo(x,y)});cx.stroke()};
 line(P,'#fbbf24');line(G.i,'#a3e635');line(G.v,'#22d3ee');
 if(hov>=0&&hov<n){const x=X(hov);cx.strokeStyle='#94a3b8';cx.lineWidth=1;cx.beginPath();cx.moveTo(x,0);cx.lineTo(x,h);cx.stroke();
  const ago=(n-1-hov)*G.step,txt='-'+(ago>=60?Math.floor(ago/60)+'m'+pad(ago%60)+'s':ago+'s')+'  '+G.v[hov].toFixed(2)+'V  '+G.i[hov].toFixed(3)+'A  '+P[hov].toFixed(2)+'W';
  cx.font='12px system-ui';cx.direction='ltr';cx.textAlign='left';const tw=cx.measureText(txt).width+12,tx=Math.min(Math.max(x-tw/2,0),w-tw);
  cx.fillStyle='rgba(15,23,42,.92)';cx.fillRect(tx,2,tw,20);cx.fillStyle='#e5e7eb';cx.fillText(txt,tx+6,16)}}
cv.onpointermove=cv.onpointerdown=e=>{const r=cv.getBoundingClientRect(),N=gCap(),n=G.v.length;const k=Math.round((e.clientX-r.left)/r.width*(N-1))-(N-n);hov=(k>=0&&k<n)?k:-1;draw()};
cv.onpointerleave=()=>{hov=-1;draw()};
$('gRange').onchange=()=>{G.range=+$('gRange').value;try{localStorage.setItem('gRange',G.range)}catch(e){}loadHist()};

// ---------- status poll ----------
function render(d,fresh){if(!d)return;lastOk=d;R=d.r;const lp=$('link');lp.textContent=t(d.ok?'online':'noReply')+(d.ok&&typeof wsLive==='function'&&wsLive()?' ⚡':'');lp.title=wsRate?wsRate.toFixed(1)+'/s':'';lp.className='pill '+(d.ok?'ok':'bad');
 const vo=R[2]/100,io=R[3]/1000,po=R[4]/100;
 $('vo').textContent=vo.toFixed(2);$('io').textContent=io.toFixed(3);$('po').textContent=po.toFixed(2);$('vin').textContent=(R[5]/100).toFixed(2);
 {const vin=R[5]/100,EFF=0.9,ia=po/EFF/Math.max(vin,1),pct=Math.round(ia*100/ADAPTER_A),e=$('iinEst');
  e.textContent=(po>0.05&&vin>1)?'≈'+ia.toFixed(3)+'A · '+pct+'%':'';e.style.color=pct>=90?'#fca5a5':pct>=75?'#fdba74':'';e.title=t('estHint')+' · '+ADAPTER_A+'A';
  if(pct>=90&&Date.now()-lastAdWarn>30000){lastAdWarn=Date.now();toast(t('adWarn',pct),1)}}
 $('ah').textContent=(u32(R[6],R[7])/1000).toFixed(3);$('wh').textContent=(u32(R[8],R[9])/1000).toFixed(3);
 $('tm').textContent=R[10]+':'+pad(R[11])+':'+pad(R[12]);
 $('tin').textContent=(s16(R[13])/10).toFixed(1);$('tex').textContent=(R[14]&&R[14]!==8888)?(s16(R[14])/10).toFixed(1):'--';
 if(act()!==shownAct)refreshAct();
 document.querySelectorAll('.tu').forEach(e=>e.textContent=R[19]?'°F':'°C');
 const cc=R[17]===1;$('cvcc').textContent=cc?'CC':'CV';$('cvcc').className='badge '+(cc?'cc':'cv');
 on=!!R[18];$('onTxt').textContent=on?'ON':'OFF';$('onTxt').style.background=on?'#052e16':'';$('onTxt').style.color=on?'#86efac':'';paintPower();
 const lk=!!R[15];if(lk!==locked){locked=lk;applyLock()}else $('pwr').disabled=locked;
 const pr=R[16];if(pr){const n=PROT[pr]||('code '+pr);$('alert').style.display='block';$('alTitle').textContent=t('protTrip')+n;$('alText').textContent=t('P')[n]||''}else $('alert').style.display='none';
 if(Date.now()>editUntil&&document.activeElement!==vN&&document.activeElement!==iN){vS.value=R[0];iS.value=R[1];vN.value=(R[0]/100).toFixed(2);iN.value=(R[1]/1000).toFixed(3)}
 if(fresh)addLive(vo,io);else draw();
 const ext=d.ext>=17;$('noExt').style.display=ext?'none':'block';['cpCard','mpCard','bfCard'].forEach(c=>$(c).style.display=ext?'':'none');
 renderJobs(d);
 if(!sysFilled&&d.ok&&R[22]){sysFilled=true;fillSys();fillModes()}
 devInfo()}
let pollMs=700,lastAdWarn=0,ADAPTER_A=2.1;
try{ADAPTER_A=+(localStorage.getItem('adA')||2.1)||2.1}catch(e){}
$('adA').value=ADAPTER_A;$('adA').onchange=()=>{const v=+$('adA').value;if(v>0){ADAPTER_A=v;try{localStorage.setItem('adA',v)}catch(e){}toast(t('saved'))}};
// Live data: WebSocket push on port 81; HTTP polling only while the socket is down or silent.
let ws=null,wsOk=false,wsLast=0,wsRate=0,wsN=0,wsT0=0;
function wsStart(){if(ws)return;try{ws=new WebSocket('ws://'+location.host+'/ws')}catch(e){ws=null;return setTimeout(wsStart,5000)}
 ws.onopen=()=>{wsOk=true;wsN=0;wsT0=Date.now()};
 ws.onmessage=e=>{wsLast=Date.now();wsN++;if(wsLast-wsT0>3000){wsRate=wsN*1000/(wsLast-wsT0);wsN=0;wsT0=wsLast}try{render(JSON.parse(e.data),1)}catch(x){}};
 ws.onclose=ws.onerror=()=>{if(!ws)return;try{ws.close()}catch(x){}ws=null;wsOk=false;setTimeout(wsStart,5000)}}
const wsLive=()=>wsOk&&Date.now()-wsLast<2000;
async function poll(){if(wsLive())return setTimeout(poll,1000);
 try{const r=await fetch('/status');const d=await r.json();pollMs=d.fast?300:700;render(d,1)}catch(e){const lp=$('link');lp.textContent=t('wifiLost');lp.className='pill bad'}setTimeout(poll,pollMs)}
wsStart();

// ---------- jobs: timer / program / charge / soft start ----------
const mmss=s=>Math.floor(s/60)+':'+pad(s%60), hms=s=>Math.floor(s/3600)+':'+pad(Math.floor(s/60)%60)+':'+pad(s%60);
let J={};
function renderJobs(d){J=d;const b=[];
 if(d.ramp)b.push(['#083344','var(--v)',t('jobRamp')]);
 if(d.timer)b.push(['#3f1d0b','#fdba74','⏱ '+t('jobTimer',mmss(d.tOff))]);
 if(d.prog)b.push(['#1e1b4b','#c7d2fe','▶ '+t('jobProg',d.prog,d.progN,d.progLeft)]);
 if(d.chg)b.push(['#052e16','#86efac','🔋 '+t('jobChg',d.chgMah)]);
 $('jobLine').innerHTML=b.map(x=>'<span class="badge" style="background:'+x[0]+';color:'+x[1]+'">'+x[2]+'</span>').join('');
 $('jobLine').style.display=b.length?'flex':'none';
 $('progBtn').textContent=t(d.prog?'stopProg':'runProg');$('progBtn').className='b '+(d.prog?'bad':'ok');
 if(d.chg){const pct=d.chgCap?Math.round(d.chgMah*100/d.chgCap):0;$('bProg').textContent=t('chgProg',d.chgMah,pct,hms(d.chgSec),R[17]===1?'CC':'CV')}else $('bProg').textContent='';
 if(d.time){const dt=new Date(d.time*1000);$('devTime').textContent=dt.toISOString().slice(0,19).replace('T',' ')}}
async function setTimer(sec){try{await api('/timer?sec='+sec);toast(t(sec?'timerSet':'timerOff'))}catch(e){toast(e.message,1)}}
(function(){$('tmrChips').innerHTML=[1,5,15,30,60].map(m=>'<button onclick="setTimer('+m*60+')"><b>'+m+'</b><span class="tmin"></span></button>').join('')+'<button onclick="setTimer(0)"><b>✕</b><span data-t="cancel"></span></button>'})();

// ---------- options (/opts) ----------
let O={};
const hhmm=m=>m<0?'':pad(Math.floor(m/60))+':'+pad(m%60), toMin=s=>{if(!s)return -1;const p=s.split(':');return (+p[0])*60+(+p[1])};
async function loadOpts(){try{O=JSON.parse(await api('/opts'));
 $('softS').value=O.soft/1000;$('schEn').value=O.schedEn;$('schOn').value=hhmm(O.schedOn);$('schOff').value=hhmm(O.schedOff);
 $('clkEn').value=O.clock;$('tzH').value=O.tz/60;$('clkNo').style.display=O.clkOk===0?'block':'none';
 $('tgEn').value=O.tgEn;$('tgTok').value=O.tgToken;$('tgChat').value=O.tgChat;$('tgLast').textContent=t('tgLastL')+O.tgLast;
 $('mqEn').value=O.mqEn;$('mqHost').value=O.mqHost;$('mqPort').value=O.mqPort;$('mqUser').value=O.mqUser;$('mqPass').value=O.mqPass;$('mqBase').value=O.mqBase;
 $('mqSt').textContent=t(O.mqOk?'mqOn':'mqOff');$('mqSt').className='pill '+(O.mqOk?'ok':'');
 $('stRep').value=O.repeat;steps=O.steps.map(s=>[s[0]/100,s[1]/1000,s[2]]);drawSteps()}catch(e){}}
async function setOpts(q,msg){try{O=JSON.parse(await api('/opts?'+q));toast(msg||t('saved'));loadOpts()}catch(e){toast(t('failed')+': '+e.message,1)}}
const enc=encodeURIComponent;
function saveSoft(){const s=+$('softS').value;if(!(s>=0&&s<=60))return toast('0–60',1);setOpts('soft='+Math.round(s*1000))}
function saveSched(){setOpts('schedEn='+$('schEn').value+'&schedOn='+toMin($('schOn').value)+'&schedOff='+toMin($('schOff').value))}
function saveClock(){setOpts('clock='+$('clkEn').value+'&tz='+Math.round(+$('tzH').value*60))}
function saveTg(){setOpts('tgEn='+$('tgEn').value+'&tgToken='+enc($('tgTok').value.trim())+'&tgChat='+enc($('tgChat').value.trim()))}
async function testTg(){toast(t('sending'));try{const r=await api('/notify-test');toast(t('tgLastL')+r)}catch(e){toast(t('tgLastL')+e.message,1)}loadOpts()}
function saveMq(){setOpts('mqEn='+$('mqEn').value+'&mqHost='+enc($('mqHost').value.trim())+'&mqPort='+(+$('mqPort').value||1883)+'&mqUser='+enc($('mqUser').value)+'&mqPass='+enc($('mqPass').value)+'&mqBase='+enc($('mqBase').value.trim()||'sk120'))}

// ---------- step program ----------
let steps=[];
function drawSteps(){$('stepTbl').innerHTML='<tr><th>#</th><th>V</th><th>A</th><th>s</th><th></th></tr>'+steps.map((s,k)=>
 '<tr><td>'+(k+1)+'</td><td><input type="number" step="0.01" min="0" max="36" value="'+s[0]+'" onchange="steps['+k+'][0]=+this.value"></td><td><input type="number" step="0.001" min="0" max="6" value="'+s[1]+'" onchange="steps['+k+'][1]=+this.value"></td><td><input type="number" min="1" max="65535" value="'+s[2]+'" onchange="steps['+k+'][2]=+this.value"></td><td><button class="b" style="padding:6px 9px" onclick="steps.splice('+k+',1);drawSteps()">✕</button></td></tr>').join('')}
function addStep(){if(steps.length>=10)return toast(t('maxSteps'),1);const l=steps[steps.length-1]||[5,0.5,60];steps.push([l[0],l[1],l[2]]);drawSteps()}
function stepsQuery(){for(const s of steps)if(!(s[0]>=0&&s[0]<=36&&s[1]>=0&&s[1]<=6&&s[2]>=1&&s[2]<=65535)){toast(t('limVI'),1);return null}
 return 'steps='+enc(steps.map(s=>Math.round(s[0]*100)+','+Math.round(s[1]*1000)+','+Math.round(s[2])).join(';'))+'&repeat='+(+$('stRep').value||0)}
function saveSteps(){const q=stepsQuery();if(q!==null)setOpts(q)}
async function toggleProg(){if(J.prog){try{await api('/prog?run=0')}catch(e){toast(e.message,1)}return}
 const q=stepsQuery();if(q===null)return;if(!steps.length)return toast(t('maxSteps'),1);
 try{await api('/opts?'+q);await api('/prog?run=1')}catch(e){toast(t('failed')+': '+e.message,1)}}

// ---------- battery charger ----------
const BAT={li:[4.20,0.05],lfp:[3.65,0.05],pb:[2.40,0.02]};
function batPlan(){const p=BAT[$('bType').value],cells=Math.max(1,Math.round(+$('bCells').value)),cap=Math.max(100,+$('bCap').value||0),rate=+$('bRate').value;
 const V=+(p[0]*cells).toFixed(2),I=+Math.min(6,cap/1000*rate).toFixed(3),cut=+Math.max(0.01,cap/1000*p[1]).toFixed(3);return {V,I,cut,cap}}
function batCalc(){const b=batPlan();$('bCalc').textContent=b.V>36?t('batTooHigh'):t('batCalc',b.V.toFixed(2),b.I.toFixed(3),b.cut.toFixed(3))}
['bType','bCells','bCap','bRate'].forEach(id=>$(id).addEventListener('input',batCalc));
async function startCharge(){if(locked)return toast(t('devLocked'),1);const b=batPlan();if(b.V>36)return toast(t('batTooHigh'),1);
 if(!confirm(t('batQ',b.V.toFixed(2),b.I.toFixed(3))))return;
 try{const n=act(),m=JSON.parse(await api('/mem?n='+n));const oah=Math.round(b.cap*1.3);
  m[0]=Math.round(b.V*100);m[1]=Math.round(b.I*1000);m[3]=Math.round(Math.min(38,b.V+0.3)*100);m[4]=Math.round(Math.min(6.2,b.I*1.2+0.05)*1000);m[8]=oah%65536;m[9]=Math.floor(oah/65536);
  await api('/wm?a='+(0x50+n*16)+'&v='+m.join(','));
  if(J.ext>=17){await api('/w?a='+0x22+'&v=0');await api('/w?a='+0x21+'&v='+Math.round(b.cut*1000))}
  await api('/set?v='+b.V.toFixed(2)+'&i='+b.I.toFixed(3));
  await api('/charge?run=1&cap='+b.cap);toast(t('chgStarted'))}catch(e){toast(t('failed')+': '+e.message,1)}}
async function stopCharge(){try{await api('/charge?run=0')}catch(e){toast(e.message,1)}}
addEventListener('resize',draw);

// ---------- modes ----------
function fillModes(){if(!R.length)return;$('cpSw').value=R[0x22]?1:0;$('cpW').value=(R[0x23]/10).toFixed(1);$('mpSw').value=R[0x1F]?1:0;$('mpK').value=(R[0x20]/100).toFixed(2);$('bf').value=(R[0x21]/1000).toFixed(3)}
async function saveCP(){const w=+$('cpW').value;if(!(w>=0&&w<=120))return toast('0–120 W',1);if(await wr(0x23,w*10)&&await wr(0x22,+$('cpSw').value))toast(t('cpSaved'))}
async function saveMPPT(){const k=+$('mpK').value;if(!(k>0&&k<=1))return toast('K 0–1',1);if(await wr(0x20,k*100)&&await wr(0x1F,+$('mpSw').value))toast(t('mpSaved'))}
async function saveBF(){const a=+$('bf').value;if(!(a>=0&&a<=6))return toast('0–6 A',1);if(await wr(0x21,a*1000))toast(t('saved'))}

// ---------- system ----------
function fillSys(){if(!R.length)return;$('sBl').value=R[0x14];$('sSl').value=R[0x15];$('sBz').value=R[0x1C]?1:0;$('sFc').value=R[0x13]?1:0;
 $('sTo').value=(s16(R[0x1A])/10).toFixed(1);$('sTe').value=(s16(R[0x1B])/10).toFixed(1);if(R[0x19]<BAUDS.length)$('bdSel').value=R[0x19]}
async function saveSys(){const to=Math.round(+$('sTo').value*10),te=Math.round(+$('sTe').value*10);
 const ok=await wr(0x14,+$('sBl').value)&&await wr(0x15,+$('sSl').value)&&await wr(0x1C,+$('sBz').value)&&await wr(0x13,+$('sFc').value)&&await wr(0x1A,to&0xFFFF)&&await wr(0x1B,te&0xFFFF);if(ok)toast(t('sysSaved'))}
function table(id,rows){$(id).innerHTML=rows.map(r=>'<tr><td class="mut">'+r[0]+'</td><td class="ltr">'+r[1]+'</td></tr>').join('')}
function devInfo(){if(!R.length)return;table('devInfo',[[t('model'),R[0x16]],[t('skFw'),R[0x17]],[t('mbAddr'),R[0x18]],[t('baudDev'),BAUDS[R[0x19]]||R[0x19]],[t('baudEsp'),info.baud||'-'],[t('espFw'),info.fw||'-'],[t('linkStat'),(info.mbOk??'-')+' / '+(info.mbErr??'-')]])}
async function setBaud(){const c=+$('bdSel').value;if(!confirm(t('baudQ',BAUDS[c])))return;try{await api('/baud?c='+c);toast(t('baudOk'))}catch(e){toast(t('failed'),1)}}
async function setId(){try{await api('/modbus?id='+$('mbId').value);toast(t('idSaved'))}catch(e){toast(e.message,1)}}
async function rawRead(){const a=parseInt($('rgA').value||'0'),n=parseInt($('rgN').value||'1');try{const r=JSON.parse(await api('/r?a='+a+'&n='+n));
 $('rgOut').innerHTML='<tr><th>addr</th><th>dec</th><th>hex</th></tr>'+r.map((v,k)=>'<tr><td>0x'+(a+k).toString(16).padStart(4,'0')+'</td><td>'+v+'</td><td>0x'+v.toString(16).padStart(4,'0')+'</td></tr>').join('')}catch(e){toast(t('readFail')+': '+e.message,1)}}
async function rawWrite(){const a=parseInt($('rgWA').value),v=parseInt($('rgWV').value);if(!isFinite(a)||!isFinite(v))return toast(t('addrVal'),1);
 if(!confirm(t('rawQ',v,'0x'+a.toString(16))))return;try{await api('/w?a='+a+'&v='+v);toast(t('written'))}catch(e){toast(e.message,1)}}

// ---------- network ----------
async function loadInfo(){try{info=JSON.parse(await api('/info'));const up=info.up,tt=Math.floor(up/3600)+'h '+pad(Math.floor(up/60)%60)+'m';
 table('netInfo',[[t('ap'),info.apSsid+' — '+info.apIp],[t('router'),(info.staSsid||'-')+(info.staIp?' — '+info.staIp:' '+t('notConn'))],[t('signal'),info.rssi?info.rssi+' dBm':'-'],[t('host'),'<a href="http://'+info.host+'.local">'+info.host+'.local</a>'],['MAC',info.mac],[t('heap'),info.heap+' B'],[t('uptime'),tt]]);
 if(document.activeElement!==$('apS'))$('apS').value=info.apSsid;if(document.activeElement!==$('mbId'))$('mbId').value=info.id;devInfo()}catch(e){}}
async function scanWifi(){const s=$('ssidList');s.innerHTML='<option value="">'+t('scanning')+'</option>';try{const l=JSON.parse(await api('/scan'));
 s.innerHTML=l.sort((a,b)=>b.rssi-a.rssi).map(n=>{const o=document.createElement('option');o.value=n.ssid;o.textContent=n.ssid+' ('+n.rssi+' dBm'+(n.sec?' 🔒':'')+')';return o.outerHTML}).join('')}catch(e){s.innerHTML='<option value="">'+t('scanFail')+'</option>'}}
async function connectSta(){const ss=$('ssidList').value;if(!ss)return toast(t('pickNet'),1);try{await api('/wifi?mode=sta&ssid='+encodeURIComponent(ss)+'&pass='+encodeURIComponent($('staPass').value));toast(t('connecting2'));setTimeout(loadInfo,6000)}catch(e){toast(t('failed'),1)}}
async function saveAp(){const s=$('apS').value,p=$('apP').value;if(p&&p.length<8)return toast(t('passLen'),1);if(!confirm(t('apQ',s)))return;
 try{await api('/ap?ssid='+encodeURIComponent(s)+'&pass='+encodeURIComponent(p));toast(t('apSaved'))}catch(e){toast(e.message,1)}}

applyLang();loadHist().then(poll);loadOpts();batCalc();
</script>
</body>
</html>
)rawliteral";
