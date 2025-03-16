@echo off
emmcdl -l
set /p com=COM=
emmcdl -p %com% -f MPRG8916.mbn -i 8x10_msimage.mbn
emmcdl -p %com% -f MPRG8916.mbn -r
pause