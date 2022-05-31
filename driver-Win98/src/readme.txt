1. install C compiler - e.g. Visual CPP 5.00.7022
2. install VToolsD (NuMega) - e.g. SoftICE Driver Suite 2.0.1
	it should add itself to env variables which are used in make file for includes
	in case of issues - try specifying full path to nmake.exe inside make.bat 
	and check autoexec.bat file, possibly adding compiler/IDE/other required lib folders to system PATH variable etc
3. compile by running make.bat - i had to copy driver src files to VC compiler root folder (where bin, lib etc resides)
4. output is a softat.vxd file, build is logged to res.txt file