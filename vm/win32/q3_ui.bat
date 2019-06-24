rem make sure we have a safe environement
set LIBRARY=
set INCLUDE=

mkdir build vm\ui

set cc=bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui %1

bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_main.c -o build/ui_main.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_cdkey.c -o build/ui_cdkey.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_ingame.c -o build/ui_ingame.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_serverinfo.c -o build/ui_serverinfo.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_confirm.c -o build/ui_confirm.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_setup.c -o build/ui_setup.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/../game/bg_misc.c -o build/bg_misc.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/../game/bg_lib.c -o build/bg_lib.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/../game/q_math.c -o build/q_math.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/../game/q_shared.c -o build/q_shared.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_gameinfo.c -o build/ui_gameinfo.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_atoms.c -o build/ui_atoms.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_connect.c -o build/ui_connect.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_controls2.c -o build/ui_controls2.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_demo2.c -o build/ui_demo2.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_mfield.c -o build/ui_mfield.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_credits.c -o build/ui_credits.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_menu.c -o build/ui_menu.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_options.c -o build/ui_options.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_display.c -o build/ui_display.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_sound.c -o build/ui_sound.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_network.c -o build/ui_network.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_playermodel.c -o build/ui_playermodel.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_players.c -o build/ui_players.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_playersettings.c -o build/ui_playersettings.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_preferences.c -o build/ui_preferences.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_qmenu.c -o build/ui_qmenu.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_servers2.c -o build/ui_servers2.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_sparena.c -o build/ui_sparena.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_specifyserver.c -o build/ui_specifyserver.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_splevel.c -o build/ui_splevel.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_sppostgame.c -o build/ui_sppostgame.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_startserver.c -o build/ui_startserver.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_team.c -o build/ui_team.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_video.c -o build/ui_video.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_cinematics.c -o build/ui_cinematics.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_spskill.c -o build/ui_spskill.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_addbots.c -o build/ui_addbots.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_removebots.c -o build/ui_removebots.asm
@if errorlevel 1 goto quit
rem bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_loadconfig.c -o build/ui_loadconfig.asm
rem @if errorlevel 1 goto quit
rem bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_saveconfig.c -o build/ui_saveconfig.asm
rem @if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_teamorders.c -o build/ui_teamorders.asm
@if errorlevel 1 goto quit
bin\lcc -DQ3_VM -S -Wf-target=bytecode -Wo-lccdir=bin -Wf-g -I../../code/cgame -I../../code/game -I../../code/q3_ui ../../code/q3_ui/ui_mods.c -o build/ui_mods.asm
@if errorlevel 1 goto quit

bin\q3asm -f q3_ui.q3asm -o vm/ui/ui
:quit

