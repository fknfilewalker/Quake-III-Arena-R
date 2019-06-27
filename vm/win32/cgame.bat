rem make sure we have a safe environement
set LIBRARY=
set INCLUDE=

mkdir build vm

set cc=bin\lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/ui %1

%cc% ../../code/game/bg_misc.c -o build/bg_misc.asm
@if errorlevel 1 goto quit
%cc% ../../code/game/bg_pmove.c -o build/bg_pmove.asm
@if errorlevel 1 goto quit
%cc% ../../code/game/bg_slidemove.c -o build/bg_slidemove.asm
@if errorlevel 1 goto quit
%cc% ../../code/game/bg_lib.c -o build/bg_lib.asm
@if errorlevel 1 goto quit
%cc% ../../code/game/q_math.c -o build/q_math.asm
@if errorlevel 1 goto quit
%cc% ../../code/game/q_shared.c -o build/q_shared.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_consolecmds.c -o build/cg_consolecmds.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_draw.c -o build/cg_draw.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_drawtools.c -o build/cg_drawtools.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_effects.c -o build/cg_effects.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_ents.c -o build/cg_ents.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_event.c -o build/cg_event.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_info.c -o build/cg_info.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_localents.c -o build/cg_localents.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_main.c -o build/cg_main.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_marks.c -o build/cg_marks.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_players.c -o build/cg_players.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_playerstate.c -o build/cg_playerstate.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_predict.c -o build/cg_predict.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_scoreboard.c -o build/cg_scoreboard.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_servercmds.c -o build/cg_servercmds.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_snapshot.c -o build/cg_snapshot.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_view.c -o build/cg_view.asm
@if errorlevel 1 goto quit
%cc% ../../code/cgame/cg_weapons.c -o build/cg_weapons.asm
@if errorlevel 1 goto quit


bin\q3asm -f cgame.q3asm -o vm/cgame
del vm\cgame.map
:quit
