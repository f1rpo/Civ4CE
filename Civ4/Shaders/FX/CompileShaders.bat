
set FXC= fxc.exe
set Outdir=..\FXO

md %outdir%

set InputName=Water
%FXC% /nologo /T fx_2_0 /Fo%Outdir%\%InputName%.fx  %InputName%.fx
set InputName=Civ4FlagDecal
%FXC% /nologo /T fx_2_0 /Fo%Outdir%\%InputName%.fx  %InputName%.fx
set InputName=Civ4Leaderheadshader
%FXC% /nologo /T fx_2_0 /Fo%Outdir%\%InputName%.fx  %InputName%.fx
set InputName=Civ4Mech
%FXC% /nologo /T fx_2_0 /Fo%Outdir%\%InputName%.fx  %InputName%.fx
set InputName=Civ4SkinningTeamColor
%FXC% /nologo /T fx_2_0 /Fo%Outdir%\%InputName%.fx  %InputName%.fx
set InputName=Civ4Wave
%FXC% /nologo /T fx_2_0 /Fo%Outdir%\%InputName%.fx  %InputName%.fx
set InputName=ContourShader
%FXC% /nologo /T fx_2_0 /Fo%Outdir%\%InputName%.fx  %InputName%.fx
set InputName=CultureBOrderShader
%FXC% /nologo /T fx_2_0 /Fo%Outdir%\%InputName%.fx  %InputName%.fx
set InputName=PlotIndicator
%FXC% /nologo /T fx_2_0 /Fo%Outdir%\%InputName%.fx  %InputName%.fx
set InputName=River
%FXC% /nologo /T fx_2_0 /Fo%Outdir%\%InputName%.fx  %InputName%.fx
set InputName=Symbols
%FXC% /nologo /T fx_2_0 /Fo%Outdir%\%InputName%.fx  %InputName%.fx
set InputName=Terrain_splatTile
%FXC% /nologo /T fx_2_0 /Fo%Outdir%\%InputName%.fx  %InputName%.fx
set InputName=Civ4Bloom
%FXC% /nologo /T fx_2_0 /Fo%Outdir%\%InputName%.fx  %InputName%.fx
set InputName=Civ4TorusFur
%FXC% /nologo /T fx_2_0 /Fo%Outdir%\%InputName%.fx  %InputName%.fx

PAUSE