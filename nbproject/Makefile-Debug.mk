#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/processutils.o \
	${OBJECTDIR}/src/shiva.o \
	${OBJECTDIR}/src/uprollutils.o


# C Compiler Flags
CFLAGS=-m64 -Wall -Wextra -Wpedantic

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=`pkg-config --libs libbsd` lib/libargparse.a  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/shiva

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/shiva: lib/libargparse.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/shiva: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/shiva ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/src/processutils.o: src/processutils.c
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -include debug.h `pkg-config --cflags libbsd` -std=c11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/processutils.o src/processutils.c

${OBJECTDIR}/src/shiva.o: src/shiva.c
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -include debug.h `pkg-config --cflags libbsd` -std=c11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/shiva.o src/shiva.c

${OBJECTDIR}/src/uprollutils.o: src/uprollutils.c
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -g -Wall -include debug.h `pkg-config --cflags libbsd` -std=c11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/uprollutils.o src/uprollutils.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
