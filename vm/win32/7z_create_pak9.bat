rem make sure we have a safe environement

del pak9.pk3

q3_ui.bat & cgame.bat & "C:\Program Files\7-Zip\7z.exe" -mx0 a -tzip "pak9.pk3" -r vm

:quit

