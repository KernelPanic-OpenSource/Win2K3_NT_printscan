#-------------------------------------------------------------
# 
# MINI-DRIVER Make file template
#
#-------------------------------------------------------------

ROOT=..\..\..\..\..\..
SRCDIR=.
C16ONLY=Y

#-------------------------------------------------------------
# The follwing entries should updated
#-------------------------------------------------------------
#
#-------------------------------------------------------------
# Enter the DRV file name (without extension) after DRVNAME =
#-------------------------------------------------------------
DRVNAME = STARESMS
DRVDESC = Star ESC/P Printers

#-------------------------------------------------------------
# Enter the names of all of the resident font file names
# (with extensions) after FONTS =
#-------------------------------------------------------------
FONTS = PFM\*.pfm

#-------------------------------------------------------------
# Enter the names of all of the version resource files
# (with extensions) after RCV =
#-------------------------------------------------------------
RCV = STARESMS.RCV

#-------------------------------------------------------------
# Enter the names of all of the character translation tables
# (with extensions) after CTTS =
#-------------------------------------------------------------
CTTS = 

#-------------------------------------------------------------
# These are the functions implemented in $(DRVNAME).c
# used to replace the ones in minidriv.c
#-------------------------------------------------------------
NOFUNCS =


#-------------------------------------------------------------
# Extra exported functions
#-------------------------------------------------------------
EXP1 =

#**********************************************************************
# Do not edit below this line
#**********************************************************************

!include ..\minidriv.mk
!include ..\default.mk

