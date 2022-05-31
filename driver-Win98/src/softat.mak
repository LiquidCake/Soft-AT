# SOFTAT.MAK - makefile for VxD SoftAT

DEVICENAME = SOFTAT
FRAMEWORK = C
DEBUG = 1
OBJECTS = SOFTAT.OBJ
DYNAMIC = 1

!include $(VTOOLSD)\include\vtoolsd.mak
!include $(VTOOLSD)\include\vxdtarg.mak

SOFTAT.OBJ:	SOFTAT.C SOFTAT.H
